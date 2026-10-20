#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QSqlDatabase>
#include <QString>

enum class UserRole {
    None,
    Admin,
    Student,
    Professor

};
struct LoginResult {
    UserRole role = UserRole::None;

    // Student
    int     studentDbId = 0;
    QString studentUniId;

    // Professor
    int     professorDbId = 0;
    QString professorCode;
};



class QLineEdit;
class QPushButton;

class Login : public QDialog
{
    Q_OBJECT
public:
    explicit Login(const QSqlDatabase &db, QWidget *parent = nullptr);

    LoginResult result() const { return m_result; }

private slots:
    void onLoginClicked();

private:
    QSqlDatabase m_db;

    QLineEdit   *m_userEdit = nullptr;
    QLineEdit   *m_passEdit = nullptr;
    QPushButton *m_loginBtn = nullptr;

    LoginResult  m_result;
};

#endif 
