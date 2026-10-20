#include "login.h"
#include "professorwindow.h"

#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

Login::Login(const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent),
    m_db(db)
{
    setWindowTitle("Login");
    setModal(true);
    resize(350, 150);

    
    m_userEdit = new QLineEdit(this);
    m_userEdit->setPlaceholderText("Student code or ADMIN");

    m_passEdit = new QLineEdit(this);
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPlaceholderText("Password ");

    m_loginBtn = new QPushButton("Login", this);

    QFormLayout *form = new QFormLayout;
    form->addRow("Username / ID:", m_userEdit);
    form->addRow("Password:",      m_passEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(form);
    mainLayout->addWidget(m_loginBtn);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(m_loginBtn, &QPushButton::clicked,
            this, &Login::onLoginClicked);
}

void Login::onLoginClicked()
{
    const QString user = m_userEdit->text().trimmed();
    const QString pass = m_passEdit->text();

    if (user.isEmpty()) {
        QMessageBox::warning(this, "Login failed",
                             "Please enter ADMIN or a student code.");
        return;
    }

    // =========================
    //   1) Admin login
    // =========================
    if (user.compare("ADMIN", Qt::CaseInsensitive) == 0) {
        if (pass == "0000") {
            m_result.role = UserRole::Admin;
            m_result.studentDbId = 0;
            m_result.studentUniId.clear();
            accept();   // dialog = OK
        } else {
            QMessageBox::warning(this, "Login failed",
                                 "Wrong admin password.");
        }
        return;
    }



    // ===================
    // 2) Professor login
    // ===================
    QSqlQuery p(m_db);
    p.prepare("SELECT id, professor_id FROM Professors WHERE professor_id = :pid");
    p.bindValue(":pid", user);

    if (!p.exec()) {
        QMessageBox::critical(this, "DB Error",
            "Failed to check professor ID:\n" + p.lastError().text());
        return;
    }
    if (p.next()) {
        m_result.role = UserRole::Professor;

        m_result.professorDbId = p.value(0).toInt();   // id (PK)
        m_result.professorCode = p.value(1).toString(); // professor_id

        accept();
        return;
    }






    // =========================
    //   3) Student login
    // =========================
    // مفيش باسورد، بس بنتأكد ان الكود موجود فى جدول Students
    QSqlQuery q(m_db);
    q.prepare("SELECT id, student_id FROM Students WHERE student_id = :sid");
    q.bindValue(":sid", user);

    if (!q.exec()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to check student ID:\n" + q.lastError().text());
        return;
    }

    if (q.next()) {
        m_result.role         = UserRole::Student;
        m_result.studentDbId  = q.value(0).toInt();
        m_result.studentUniId = q.value(1).toString();
        accept();   
    } else {
        QMessageBox::warning(this, "Login failed",
                             "Student ID not found.");
    }
}


