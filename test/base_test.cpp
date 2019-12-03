#include "base_test.h"

#include <QtTest/QtTest>
#include <QTextCursor>


void BaseTest::addColumns() {
    QTest::addColumn<QString>("origin");
    QTest::addColumn<CursorPos>("cursorPos");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
}

void BaseTest::setCursorPosition(int line, int col) {
    QTextCursor cursor(qpart.document()->findBlockByNumber(line));
    cursor.setPosition(cursor.block().position() + col);
    qpart.setTextCursor(cursor);
}

void BaseTest::enter() {
    QTest::keyClick(&qpart, Qt::Key_Enter);
}

void BaseTest::tab() {
    QTest::keyClick(&qpart, Qt::Key_Tab);
}

void BaseTest::type(const QString& text) {
    QTest::keyClicks(&qpart, text);
}

void BaseTest::verifyExpected(const QString& expected) {
    QCOMPARE(qpart.toPlainText(), expected);
}

void BaseTest::runDataDrivenTest() {
    QFETCH(QString, origin);
    QFETCH(CursorPos, cursorPos);
    QFETCH(QString, input);
    QFETCH(QString, expected);

    qpart.setPlainText(origin);

    setCursorPosition(cursorPos.first, cursorPos.second);

#if 1  // xml splitting not supported now
    if (input.startsWith("<<alignLine(")) {
        int blockIndex = input.mid(QString("<<alignLine(").length(), 1).toInt();
        QTextBlock block = qpart.document()->findBlockByNumber(blockIndex);
        QTextCursor cursor(block);
        qpart.setTextCursor(cursor);
        qpart.autoIndentCurrentLine();
    } else {
#endif
        for (auto ch = input.begin(); ch != input.end(); ++ch) {
            if (*ch == '\n') {
                enter();
            } else if (*ch == '\t') {
                tab();
            } else {
                type(*ch);
            }
        }
#if 1
    }
#endif
    verifyExpected(expected);
}

void BaseTest::initTestCase() {
     Q_INIT_RESOURCE(qutepart_syntax_files);
}

void BaseTest::init() {
    qpart.setPlainText("");
}
