#pragma once

#include <QVector>
#include <QMap>
#include <QSet>
#include <QStack>
#include <QMessageBox>

enum tokentype{
	VAR, NUM, OP_BR, CL_BR, FUNC, OP, OP_SBR, CL_SBR
};

struct Token{
    QString name;
	tokentype type;
};

// Проверки символа
bool isDelimiter(char c);

// Разбиваем выражение на токены
bool createTokensFromExpression(QString & expr, QVector<Token> & tokens);

// Расставляем токены в порядке постфиксной записи
bool createPostfixFromTokens(QVector<Token> & postfix, QVector<Token> & tokens);

// Подсчет постфиксного выражения записанного токенами
double calculate(QVector<Token> postfix, double t);

// Функция рассчитывающая массив значений выражения
int getVals(QString sBi, QString sBj, QString sBk,
             QString sEi, QString sEj, QString sEk,
             double dt, int N, double qm,
             // Возвращаемые значения
             QVector<double> & t,
             QVector<double> & Mx, QVector<double> & My, QVector<double> & Mz,
             QVector<double> & Vx, QVector<double> & Vy, QVector<double> & Vz,
             QVector<double> & ax, QVector<double> & ay, QVector<double> & az);

