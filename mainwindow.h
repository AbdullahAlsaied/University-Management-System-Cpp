#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTableWidget>
#include "login.h"

// Forward declarations
class QTabWidget;
class QTableView;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // ✅ نستقبل نتيجة اللوجين
    void setCurrentUser(const LoginResult& res);

    // (اختياري) استخدام الداتابيز خارجياً
    const QSqlDatabase& database() const { return db; }

private slots:
    // ------ Students ------
    void addStudent();
    void applyStudentFilter();
    void autoAssignSections();
    void deleteStudent();

    // ------ Courses ------
    void addCourse();
    void deleteCourse();

    // ------ Grades ------
    void saveGrade();
    void deleteGrade();

private:
    // ------ Setup ------
    void initDatabase();
    void setupTabs();
    void setupStudentsTab();
    void setupCoursesTab();
    void setupGradesTab();
    void setupProfessorsTab();   // ✅ NEW

    void refreshStudentCombo();
    void refreshCourseCombo();

    // ------ Current User ------
    LoginResult currentUser;

    // ------ Database ------
    QSqlDatabase    db;
    QSqlTableModel* studentsModel = nullptr;
    QSqlTableModel* coursesModel = nullptr;
    QSqlTableModel* gradesModel = nullptr;
    QSqlTableModel* professorsModel = nullptr;   

    // ------ Tabs ------
    QTabWidget* tabWidget = nullptr;
    QWidget* studentsTab = nullptr;
    QWidget* coursesTab = nullptr;
    QWidget* gradesTab = nullptr;
    QWidget* professorsTab = nullptr;         

    // ------ Students UI ------
    QTableView* studentsView = nullptr;
    QLineEdit* studentNameEdit = nullptr;
    QLineEdit* studentIdEdit = nullptr;
    QSpinBox* studentYearSpin = nullptr;
    QComboBox* studentSectionCombo = nullptr;
    QComboBox* filterYearCombo = nullptr;
    QComboBox* filterSectionCombo = nullptr;

    // ------ Courses UI ------
    QTableView* coursesView = nullptr;
    QLineEdit* courseNameEdit = nullptr;
    QLineEdit* courseCodeEdit = nullptr;
    QSpinBox* courseCreditsSpin = nullptr;
    QSpinBox* courseYearSpin = nullptr;
    QLineEdit* coursePrereqEdit = nullptr;

    // ------ Grades UI ------
    QTableView* gradesView = nullptr;
    QComboBox* gradeStudentCombo = nullptr;
    QComboBox* gradeCourseCombo = nullptr;
    QComboBox* gradeA1Combo = nullptr;
    QComboBox* gradeA2Combo = nullptr;
    QSpinBox* gradeFinalSpin = nullptr;

    // ------ Professors UI  ------
    QTableView* professorsView = nullptr;    
    QLineEdit* professorCodeEdit = nullptr;      
    QLineEdit* professorNameEdit = nullptr;      
    QLineEdit* professorEmailEdit = nullptr;      
    QLineEdit* professorDeptEdit = nullptr;   
};

#endif
