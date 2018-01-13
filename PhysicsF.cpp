#include "PhysicsF.h"

// Множество разделителей
QSet<char> delimiters = { ' ', '+', '-', '*', '/', '^', '(', ')', '[', ']' };
QSet<QString> vars = { "x", "y", "z", "t", "e", "p" };
QSet<QString> funcs = { "sin", "cos", "asin", "acos", "tg", "ctg", "atg", "actg" };

// Проверки символа
bool isDelimiter(char c){
    return delimiters.contains(c);
}

// Разбиваем выражение на токены
bool createTokensFromExpression(QString & expr, QVector<Token> & tokens){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    QString name;

	int i = 0;
    while (i < expr.length()){
		name = "";
        if (isDelimiter(expr[i].toLatin1())){
            if (expr[i].isSpace()){ // пробелы пропускаем
				i++;
				continue;
			}
			name = expr[i];
			i++;
		}
		else{
            while (i < expr.length() && !isDelimiter(expr[i].toLatin1())){
                // Если это имя функции или переменная, то считываем полностью
				name += expr[i++];
			}
		}
		// Заносим получившийся токен в список токенов
		tokens.push_back({ name, VAR });
	}

	// Классифицируем токены
    for (QVector<Token>::iterator Ti = tokens.begin(); Ti < tokens.end(); Ti++){
        if (Ti->name.startsWith('(')){
            Ti->type = OP_BR;
			continue;
		}
        if (Ti->name.startsWith(')')){
            Ti->type = CL_BR;
			continue;
		}
        if (Ti->name.startsWith('[')){
            Ti->type = OP_SBR;
			continue;
		}
        if (Ti->name.startsWith(']')){
            Ti->type = CL_SBR;
			continue;
		}
        if (Ti->name[0].isDigit()){
            Ti->type = NUM;
			continue;
		}
        if (Ti->name[0].isLetter()){
            if ((Ti+1) < tokens.end() && (Ti+1)->name.startsWith('(')){
                Ti->type = FUNC;
				continue;
			}
		}
        if (isDelimiter(Ti->name[0].toLatin1()))
            Ti->type = OP;
	}

	// Проверяем все токены
    for (QVector<Token>::iterator Ti = tokens.begin(); Ti < tokens.end(); Ti++){
        switch (Ti->type){
		case VAR:
            // У нас есть 4 переменные x, y, z, t
            // и 2 константы e, п
            if (!vars.contains(Ti->name)){
                msgBox.setInformativeText(QString("Error: unknown var \"%1\"").arg(Ti->name));
                msgBox.exec();

				return false;
			}
			break;
		case NUM:
			// Если в число затесался посторонний символ
            bool ok;
            Ti->name.toDouble((&ok));
            if (!ok){
                msgBox.setInformativeText(QString("Error: unknown value \"%1\"").arg(Ti->name));
                msgBox.exec();

                return false;
            }
			break;
		case OP_BR:
		case CL_BR:
		case OP_SBR:
		case CL_SBR:
		case OP:
			// Для всех операторов проверяем длину
            if (Ti->name.length() > 1){
                msgBox.setInformativeText(QString("Error: unknown operator \"%1\"").arg(Ti->name));
                msgBox.exec();

				return false;
			}

			// Дополнительно проверяем '-' на префиксность
            if (Ti->name.startsWith('-') && (Ti == tokens.begin() || (Ti-1)->type == OP_BR || (Ti-1)->type == OP_SBR))
                Ti->name = "opposite";
			break;
		case FUNC:
            // У нас есть 8 различных функций
            if (!funcs.contains(Ti->name)){
                msgBox.setInformativeText(QString("Error: unknown function \"%1\"").arg(Ti->name));
                msgBox.exec();

				return false;
			}
			break;
		}
	}
	return true;
}

QMap<QString, int> priority = {
    { "+", 10 },
    { "-", 10 },
    { "*", 20 },
    { "/", 20 },
    { "^", 30 },
    { "opposite", 10 }
};

// Расставляем токены в порядке постфиксной записи
bool createPostfixFromTokens(QVector<Token> & postfix, QVector<Token> & tokens){
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);

	// Но для начала проверяем выражение на вменяемость по 5 правилам
	// 1) нельзя чтобы рядом стояли представители одного типа токенов(кроме скобок)
    for (QVector<Token>::iterator Ti = tokens.begin(); Ti < tokens.end()-1; Ti++)
        if (Ti->type == (Ti+1)->type && Ti->type != OP_BR && Ti->type != CL_BR && Ti->type != OP_SBR && Ti->type != CL_SBR){
            msgBox.setInformativeText(QString("Ошибка: Выражение \"%1 %2\" не имеет смысла").arg(Ti->name, (Ti+1)->name));
            msgBox.exec();

			return false;
		}
	// 2) выражение не может начинаться и заканчиваться оператором(кроме opposite)
    if ((tokens.begin()->type == OP && tokens.begin()->name != "opposite") || (tokens.end()-1)->type == OP){
        msgBox.setInformativeText(QString("Ошибка: выражение не может начинаться или заканчиваться оператором"));
        msgBox.exec();

		return false;
	}
	// 3) выражение должно быть сбалансировано по скобкам
	int balanceCntr = 0;
    for (QVector<Token>::iterator Ti = tokens.begin(); Ti < tokens.end() && balanceCntr >= 0; Ti++){
        if (Ti->type == OP_BR)
			balanceCntr++;
        else if (Ti->type == CL_BR)
			balanceCntr--;
	}
    if (balanceCntr){
        msgBox.setInformativeText(QString("Ошибка: выражение не сбалансировано по скобкам"));
        msgBox.exec();

		return false;
	}

	// 4) у фунций должен быть задан параметр
    for (QVector<Token>::iterator Ti = tokens.begin(); Ti < tokens.end(); Ti++){
        if (Ti->type != FUNC){ // нас интересуют только функции
			continue;
		}

		// Ищем какое либо значение внутри скобок
		bool isVal = false;
        for (QVector<Token>::iterator Tj = Ti + 1; Tj->type != CL_BR && !isVal; Tj++){
            if (Tj->type == VAR || Tj->type == NUM)
				isVal = true;
		}

		if (!isVal){
            msgBox.setInformativeText(QString("Ошибка: функция %1 должна принимать аргумент").arg(Ti->name));
            msgBox.exec();

			return false;
		}
	}
	// 5) оператор не должен 'заворачиваться' в скобку (по сути правило 2 применимое к скобкам)
    for (QVector<Token>::iterator Ti = tokens.begin(); (Ti+1) < tokens.end(); Ti++){
        if ((Ti->type == OP_BR && (Ti+1)->type == OP  && (Ti+1)->name != "opposite") || (Ti->type == OP && (Ti+1)->type == CL_BR)){
            msgBox.setInformativeText(QString("Ошибка: выражение не может начинаться или заканчиваться оператором"));
            msgBox.exec();

			return false;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Приступаем непосредственно к преобразованию выражения к постфиксной нотации
    QStack<Token> TStack;

    for (QVector<Token>::iterator Ti = tokens.begin(); Ti < tokens.end(); Ti++){
        switch (Ti->type){
		case VAR:
		case NUM:
            postfix.push_back(*Ti);
			break;
		case OP_BR:
            TStack.push(*Ti);
			break;
		case CL_BR:
			while (TStack.top().type != OP_BR){
				postfix.push_back(TStack.top());
				TStack.pop();
			}
			TStack.pop();
			break;
        case OP_SBR:
            TStack.push(*Ti);
            break;
        case CL_SBR:
            while (TStack.top().type != OP_SBR){
                postfix.push_back(TStack.top());
                TStack.pop();
            }
            TStack.pop();
            break;
		case OP:
            while (!TStack.empty() && ((TStack.top().type == OP && priority[TStack.top().name] >= priority[Ti->name]) || TStack.top().type == FUNC)){
				postfix.push_back(TStack.top());
				TStack.pop();
			}
            TStack.push(*Ti);
			break;
		case FUNC:
			while (!TStack.empty() && TStack.top().type == FUNC){
				postfix.push_back(TStack.top());
				TStack.pop();
			}
            TStack.push(*Ti);
			break;
		}
	}

	// Выгребаем стек
	while (!TStack.empty()){
		postfix.push_back(TStack.top());
		TStack.pop();
	}


	return true;
}


// Подсчет постфиксного выражения записанного токенами
double calculate(QVector<Token> postfix, double t, double x, double y, double z){
	double pi = 3.14159265;
	double e = 2.71828182;

    QStack<double> fStack;

	double a, b;

    for (QVector<Token>::iterator Pi = postfix.begin(); Pi < postfix.end(); Pi++){
        switch (Pi->type){
		case VAR:
            switch (Pi->name[0].toLatin1()){
			case 'e':
				fStack.push(e);
				break;
			case 't':
				fStack.push(t);
				break;
            case 'p':
				fStack.push(pi);
				break;
            case 'x':
                fStack.push(x);
                break;
            case 'y':
                fStack.push(y);
                break;
            case 'z':
                fStack.push(z);
                break;
			}
			break;
		case NUM:
            fStack.push(Pi->name.toDouble());
			break;
		case OP:
            switch (Pi->name[0].toLatin1()){
			case '+':
				a = fStack.top();
				fStack.pop();
				b = fStack.top();
				fStack.pop();
				a += b;
				fStack.push(a);
				break;
			case '-':
				a = fStack.top();
				fStack.pop();
				b = fStack.top();
				fStack.pop();
				b -= a;
				fStack.push(b);
				break;
			case '*':
				a = fStack.top();
				fStack.pop();
				b = fStack.top();
				fStack.pop();
				a *= b;
				fStack.push(a);
				break;
			case '/':
				a = fStack.top();
				fStack.pop();
				b = fStack.top();
				fStack.pop();
				b /= a;
				fStack.push(b);
				break;
			case '^': // pow
				a = fStack.top();
				fStack.pop();
				b = fStack.top();
				fStack.pop();
				a = pow(b, a);
				fStack.push(a);
				break;
			case 'o': // opposite
				a = fStack.top();
				fStack.pop();
				a *= -1;
				fStack.push(a);
				break;
			}
			break;
		case FUNC:
			a = fStack.top();
			fStack.pop();
            if (Pi->name == "sin"){
				fStack.push(sin(a * pi / 180));
			}
            else if (Pi->name == "cos"){
				fStack.push(cos(a * pi / 180));
			}
            else if (Pi->name == "asin"){
				fStack.push(asin(a));
			}
            else if (Pi->name == "acos"){
				fStack.push(acos(a));
			}
            else if (Pi->name == "tg"){
				fStack.push(tan(a * pi / 180));
			}
            else if (Pi->name == "ctg"){
				fStack.push(1 / tan(a * pi / 180));
			}
            else if (Pi->name == "atg"){
				fStack.push(atan(a));
			}
            else if (Pi->name == "actg"){
				fStack.push(pi / 2 - atan(a));
			}
			break;
        default:
            break;
		}
	}

	double result = fStack.top();
	fStack.pop();

	return result;
}

void getFuncFromExpr(QString & expr, QVector<Token> & func){
    QVector<Token> tmp;
    createTokensFromExpression(expr, tmp);
    createPostfixFromTokens(func, tmp);
}


// Функция рассчитывающая массив значений выражения
int getVals(QString sBi, QString sBj, QString sBk,
             QString sEi, QString sEj, QString sEk,
             double dt, int N, double qm,
             // Возвращаемы значения
             QVector<double> & t,
             QVector<double> & Mx, QVector<double> & My, QVector<double> & Mz,
             QVector<double> & Vx, QVector<double> & Vy, QVector<double> & Vz,
             QVector<double> & ax, QVector<double> & ay, QVector<double> & az)
{
    // Получяем функции в токенах из строки
    QVector<Token> Bi;
    QVector<Token> Bj;
    QVector<Token> Bk;

    getFuncFromExpr(sBi, Bi);
    getFuncFromExpr(sBj, Bj);
    getFuncFromExpr(sBk, Bk);

    QVector<Token> Ei;
    QVector<Token> Ej;
    QVector<Token> Ek;

    getFuncFromExpr(sEi, Ei);
    getFuncFromExpr(sEj, Ej);
    getFuncFromExpr(sEk, Ek);

    // Начинаем рассчет
    double Ex, Ey, Ez;
    double Bx, By, Bz;

    for (int i = 1; i < N; i++){
        t[i] = t[i-1] + dt;

        // Рассчет функций
        Ex = calculate(Ei, t[i], Mx[i-1], My[i-1], Mz[i-1]);
        Ey = calculate(Ej, t[i], Mx[i-1], My[i-1], Mz[i-1]);
        Ez = calculate(Ek, t[i], Mx[i-1], My[i-1], Mz[i-1]);

        if (_isnan(Ex) || _isnan(Ey) || _isnan(Ez))
            return i;

        Bx = calculate(Bi, t[i], Mx[i-1], My[i-1], Mz[i-1]);
        By = calculate(Bj, t[i], Mx[i-1], My[i-1], Mz[i-1]);
        Bz = calculate(Bk, t[i], Mx[i-1], My[i-1], Mz[i-1]);

        if (_isnan(Bx) || _isnan(By) || _isnan(Bz))
            return i;

        // Значения для графиков
        ax[i] = qm * (Ex + Vy[i-1]*Bz - Vz[i-1]*By);
        ay[i] = qm * (Ey + Vx[i-1]*Bz - Vz[i-1]*Bx);
        az[i] = qm * (Ez + Vx[i-1]*By - Vy[i-1]*Bx);

        if (_isnan(ax[i]) || _isnan(ay[i]) || _isnan(az[i]))
            return i;

        Vx[i] = Vx[i-1] + ax[i]*dt;
        Vy[i] = Vy[i-1] + ay[i]*dt;
        Vz[i] = Vz[i-1] + az[i]*dt;

        if (_isnan(Vx[i]) || _isnan(Vy[i]) || _isnan(Vz[i]))
            return i;

        Mx[i] = Mx[i-1] + Vx[i]*dt;
        My[i] = My[i-1] + Vy[i]*dt;
        Mz[i] = Mz[i-1] + Vz[i]*dt;

        if (_isnan(Mx[i]) || _isnan(My[i]) || _isnan(Mz[i]))
            return i;
	}

    return 0;
}

//int main(){
//	setlocale(LC_CTYPE, "rus");
//
//	string expr = "-3 + 4 - [e] * sin(2) + cos(t) / pi^(actg(t/2) - atg(-t/2)) + asin(0.5) * acos(0.4) + sin(cos(3))";
//	vector<Token> tokens;
//	vector<Token> postfix;
//	if (createTokensFromExpression(expr, tokens))
//		if (createPostfixFromTokens(postfix, tokens)){
//			for (int i = 0; i < postfix.size(); i++)
//				printf("%s ", postfix[i].name.c_str());
//
//			printf("\n");
//		}
//
//	for (double t = 9.8; t < 1000; t += 5){
//		cout << "f(" << t << ") = " << calculate(postfix, t) << endl;
//		cout << "f`(" << t << ") = " << derivative(postfix, t) << endl;
//		cout << "f``(" << t << ") = " << derivative2(postfix, t) << endl;
//		cout << endl;
//	}
//
//	return 0;
//}
