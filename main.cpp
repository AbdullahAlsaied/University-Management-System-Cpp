#include <QApplication>
#include "mainwindow.h"
#include "login.h"
#include "professorwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // نِنشئ الـ MainWindow عشان يفتح الـ DB جوّه initDatabase()
    MainWindow w;

    // نفتح شاشة الـ Login ونمرّر لها نفس الـ DB
    Login loginDlg(w.database());

    // لو اليوزر قفل الـ Login أو Cancel → نخرج من البرنامج
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;
    }


    LoginResult res = loginDlg.result();

    if (res.role == UserRole::Professor) {
        ProfessorWindow* pw = new ProfessorWindow(res.professorDbId);
        pw->show();
    }
    else {
        w.setCurrentUser(res);
        w.show();
    }



    return a.exec();

}
