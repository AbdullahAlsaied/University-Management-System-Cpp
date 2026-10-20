#include "mainwindow.h"
#include <QTabWidget>
#include <QTableView>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QLabel>
#include <QSqlRecord>


#include <QMenuBar>
#include <QMenu>
#include <QAction>

// ======  للدرجات ======
static int gradeToScore(const QString &g)
{
    const QString s = g.trimmed().toUpper();
    if (s == "NA") return 0;
    if (s == "P")  return 50;
    if (s == "M")  return 70;
    if (s == "D")  return 90;
    return 0;
}

static int calculateTotal(const QString &a1,
                          const QString &a2,
                          const QString &finalGrade)
{
    int s1 = gradeToScore(a1);
    int s2 = gradeToScore(a2);
    int sf = gradeToScore(finalGrade);
    double avg = (s1 + s2 + sf) / 3.0;
    return static_cast<int>(avg + 0.5);
}

static int applyMercy(int rawScore)
{
    int boosted = rawScore + 5;
    if (boosted > 100) boosted = 100;
    return boosted;
}

// ====== MainWindow ======
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("University Management System");
    setMinimumSize(1100, 700);

    initDatabase();
    setupTabs();

    QMenu *accountMenu = menuBar()->addMenu("Account");

    QAction *logoutAction = new QAction("Log out", this);
    accountMenu->addAction(logoutAction);

    connect(logoutAction, &QAction::triggered, this, [this]() {
        Login loginDlg(this->database(), this);

        if (loginDlg.exec() == QDialog::Accepted) {
            LoginResult res = loginDlg.result();
            setCurrentUser(res);
        }
    });
}

MainWindow::~MainWindow()
{
}
void MainWindow::setCurrentUser(const LoginResult& res)
{
    currentUser = res;

    if (!gradesModel || !tabWidget)
        return;

    //  نمسح كل التابات 
    tabWidget->clear();

    if (currentUser.role == UserRole::Admin) {
        // ================= ADMIN =================
        setWindowTitle("University Management System - ADMIN");

        setupStudentsTab();
        setupCoursesTab();
        setupProfessorsTab();
        setupGradesTab();   

        gradesModel->setFilter(QString());
        gradesModel->select();
    }
    else if (currentUser.role == UserRole::Student) {
        // ================= STUDENT =================
        setWindowTitle(
            "University Management System - Student (" +
            currentUser.studentUniId + ")"
        );

        // نمسح التابات
        tabWidget->clear();

        // ---------- Grades Tab ----------
        QWidget* gradesTab = new QWidget();
        QVBoxLayout* gradesLayout = new QVBoxLayout(gradesTab);

        QLabel* title = new QLabel("My Grades");
        title->setStyleSheet("font-size:16px; font-weight:bold;");
        gradesLayout->addWidget(title);

        QTableWidget* table = new QTableWidget();
        table->setColumnCount(8);
        table->setRowCount(3);

        table->setHorizontalHeaderLabels({
            "Course", "Type", "A1", "A2", "CW", "Final", "Total", "Grade"
            });

        table->setItem(0, 0, new QTableWidgetItem("C++"));
        table->setItem(0, 1, new QTableWidgetItem("Practical (150)"));
        table->setItem(0, 2, new QTableWidgetItem("25"));
        table->setItem(0, 3, new QTableWidgetItem("40"));
        table->setItem(0, 4, new QTableWidgetItem("30"));
        table->setItem(0, 5, new QTableWidgetItem("45"));
        table->setItem(0, 6, new QTableWidgetItem("140 / 150"));
        table->setItem(0, 7, new QTableWidgetItem("Excellent"));

        table->setItem(1, 0, new QTableWidgetItem("Database"));
        table->setItem(1, 1, new QTableWidgetItem("Practical (100)"));
        table->setItem(1, 2, new QTableWidgetItem("18"));
        table->setItem(1, 3, new QTableWidgetItem("25"));
        table->setItem(1, 4, new QTableWidgetItem("15"));
        table->setItem(1, 5, new QTableWidgetItem("30"));
        table->setItem(1, 6, new QTableWidgetItem("88 / 100"));
        table->setItem(1, 7, new QTableWidgetItem("Very Good"));

        table->setItem(2, 0, new QTableWidgetItem("OS"));
        table->setItem(2, 1, new QTableWidgetItem("Theoretical (100)"));
        table->setItem(2, 2, new QTableWidgetItem("18"));
        table->setItem(2, 3, new QTableWidgetItem("16"));
        table->setItem(2, 4, new QTableWidgetItem("—"));
        table->setItem(2, 5, new QTableWidgetItem("50"));
        table->setItem(2, 6, new QTableWidgetItem("84 / 100"));
        table->setItem(2, 7, new QTableWidgetItem("Very Good"));

        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->horizontalHeader()->setStretchLastSection(true);

        gradesLayout->addWidget(table);

        QLabel* note = new QLabel(
            "Note: Grades are displayed according to course type and evaluation rules."
        );
        note->setStyleSheet("color: gray; font-style: italic;");
        gradesLayout->addWidget(note);

        tabWidget->addTab(gradesTab, "Grades");

        // ---------- Tuition Fees Tab ----------
        QWidget* feesTab = new QWidget();
        QVBoxLayout* feesLayout = new QVBoxLayout(feesTab);

        // ===== Title =====
        QLabel* titleFees = new QLabel("Student Fees Report");
        titleFees->setStyleSheet("font-size:18px; font-weight:bold;");
        feesLayout->addWidget(titleFees);

        // ===== Summary =====
        QHBoxLayout* summaryLayout = new QHBoxLayout();
        summaryLayout->addWidget(new QLabel("Status: New Student"));
        summaryLayout->addStretch();
        summaryLayout->addWidget(new QLabel("Total: 31,371.90 EGP"));
        summaryLayout->addWidget(new QLabel("Paid: 15,600.00 EGP"));
        summaryLayout->addWidget(new QLabel("Remaining: 15,771.90 EGP"));
        feesLayout->addLayout(summaryLayout);

        // ===== Academic Info =====
        QLabel* academicInfo = new QLabel(
            "Academic Year: 2026 / 2025 | Level: Second Year | Semester: First | Program: IT"
        );
        academicInfo->setStyleSheet("font-weight:bold;");
        feesLayout->addWidget(academicInfo);

        // ===== Table =====
        QTableWidget* feesTable = new QTableWidget();
        feesTable->setColumnCount(5);
        feesTable->setRowCount(2);

        QStringList headers;
        headers << "Main Fee" << "Sub Fee" << "Amount (EGP)" << "Paid (EGP)" << "Remaining (EGP)";
        feesTable->setHorizontalHeaderLabels(headers);

        // Row 1
        feesTable->setItem(0, 0, new QTableWidgetItem("Study Fees"));
        feesTable->setItem(0, 1, new QTableWidgetItem("Second Year"));
        feesTable->setItem(0, 2, new QTableWidgetItem("15,571.90"));
        feesTable->setItem(0, 3, new QTableWidgetItem("0.00"));
        feesTable->setItem(0, 4, new QTableWidgetItem("15,571.90"));

        // Row 2
        feesTable->setItem(1, 0, new QTableWidgetItem("Second Round Fees"));
        feesTable->setItem(1, 1, new QTableWidgetItem("-"));
        feesTable->setItem(1, 2, new QTableWidgetItem("200.00"));
        feesTable->setItem(1, 3, new QTableWidgetItem("0.00"));
        feesTable->setItem(1, 4, new QTableWidgetItem("200.00"));

        // Table settings
        feesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        feesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        feesTable->setSelectionMode(QAbstractItemView::NoSelection);

        feesLayout->addWidget(feesTable);

        // ===== Footer =====
        QHBoxLayout* footerLayout = new QHBoxLayout();
        footerLayout->addWidget(new QLabel("Page 1 of 1"));
        footerLayout->addStretch();
        footerLayout->addWidget(new QLabel("Print Date: 2025/12/25"));
        feesLayout->addLayout(footerLayout);

        tabWidget->addTab(feesTab, "Tuition Fees");

    }


    else {
        // ================= FALLBACK =================
        setWindowTitle("University Management System");
        gradesModel->setFilter(QString());
        gradesModel->select();
    }
}


// ================== Database ==================
void MainWindow::initDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("university.db");

    if (!db.open()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to open database: " + db.lastError().text());
        return;
    }

    QSqlQuery q(db);

    // جدول الطلبة
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS Students (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            student_id TEXT NOT NULL UNIQUE,
            name TEXT NOT NULL,
            year INTEGER NOT NULL,
            section TEXT NOT NULL
        )
    )");

    // جدول الكورسات
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS Courses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            code TEXT NOT NULL UNIQUE,
            name TEXT NOT NULL,
            credits INTEGER NOT NULL,
            year INTEGER NOT NULL,
            prerequisites TEXT
        )
    )");

    // جدول الدرجات
    q.exec(R"(
      CREATE TABLE IF NOT EXISTS Grades (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        student_id INTEGER NOT NULL,
        course_id  INTEGER NOT NULL,
        assignment1 INTEGER NOT NULL,
        assignment2 INTEGER NOT NULL,
        cw INTEGER NOT NULL,
        final_exam INTEGER NOT NULL,
        total_score INTEGER NOT NULL,
        UNIQUE(student_id, course_id)
    )
)");


    // جدول الـ Admins
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS Admins (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL   -- ممكن تبقى hashed بعدين
        )
    )");

    // جدول Professors
    q.exec(R"(
    CREATE TABLE IF NOT EXISTS Professors (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        professor_id TEXT UNIQUE NOT NULL,
        name TEXT NOT NULL,
        email TEXT,
        department TEXT
    )
)");

    // Professors  جدول ربط الكورسات 
    q.exec(R"(
    CREATE TABLE IF NOT EXISTS CourseAssignments (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        professor_id INTEGER NOT NULL,
        course_id INTEGER NOT NULL,
        UNIQUE(professor_id, course_id)
    )
)");



    // لو مفيش ولا أدمن.. نزود واحد default
    QSqlQuery check(db);
    check.exec("SELECT COUNT(*) FROM Admins");
    if (check.next()) {
        int count = check.value(0).toInt();
        if (count == 0) {
            QSqlQuery ins(db);
            ins.prepare("INSERT INTO Admins (username, password) VALUES ('admin', 'admin123')");
            ins.exec(); // مش مشكلة لو فشل، بس غالبًا هتمشي
        }
    }

    if (q.lastError().isValid()) {
        QMessageBox::warning(this, "DB Warning",
                             "Last DB error: " + q.lastError().text());
    }


    // Professor افتراضي
    QSqlQuery checkProf(db);
    checkProf.exec("SELECT COUNT(*) FROM Professors");
    if (checkProf.next() && checkProf.value(0).toInt() == 0) {
        QSqlQuery ins(db);
        ins.exec(
            "INSERT INTO Professors (professor_id, name, email, department) "
            "VALUES ('PROF001', 'محمد رمضان', 'mohamed@nct.edu', 'c++')"
        );
    }

    if (q.lastError().isValid()) {
        QMessageBox::warning(this, "DB Warning",
            "Last DB error: " + q.lastError().text());
    }



}




// ================== Tabs ==================
void MainWindow::setupTabs()
{
    tabWidget = new QTabWidget(this);

    setupStudentsTab();
    setupCoursesTab();
    setupGradesTab();

    setCentralWidget(tabWidget);
}

// ================== Students Tab ==================
void MainWindow::setupStudentsTab()
{
    studentsTab = new QWidget(this);

    studentsView  = new QTableView(studentsTab);
    studentsModel = new QSqlTableModel(studentsTab, db);
    studentsModel->setTable("Students");
    studentsModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    studentsModel->select();

    studentsView->setModel(studentsModel);
    studentsView->horizontalHeader()->setStretchLastSection(true);
    studentsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    studentsView->setColumnHidden(0, true);


    // إدخال طالب
    studentNameEdit = new QLineEdit(studentsTab);
    studentIdEdit   = new QLineEdit(studentsTab);
    studentYearSpin = new QSpinBox(studentsTab);
    studentYearSpin->setRange(1, 4);

    //مسح طالب
    QPushButton *deleteBtn = new QPushButton("Delete Student", studentsTab);
    connect(deleteBtn, &QPushButton::clicked,
            this, &MainWindow::deleteStudent);


    //  السكاشن 
    studentSectionCombo = new QComboBox(studentsTab);
    for (int i = 1; i <= 10; ++i) {
        studentSectionCombo->addItem(QString::number(i));
    }

    QPushButton *addBtn = new QPushButton("Add Student", studentsTab);
    connect(addBtn, &QPushButton::clicked,
            this, &MainWindow::addStudent);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Name:",       studentNameEdit);
    formLayout->addRow("Student ID:", studentIdEdit);
    formLayout->addRow("Year (1-4):", studentYearSpin);
    formLayout->addRow("Section:",    studentSectionCombo);
    formLayout->addRow(addBtn);
    formLayout->addRow(deleteBtn);


    // فلاتر
    filterYearCombo = new QComboBox(studentsTab);
    filterYearCombo->addItem("All", 0);
    for (int y = 1; y <= 4; ++y)
        filterYearCombo->addItem(QString::number(y), y);

    filterSectionCombo = new QComboBox(studentsTab);
    filterSectionCombo->addItem("All", "");   // كل السكاشن

    // 🔁 سكاشن 1..10 فى الفلتر برضه
    for (int i = 1; i <= 10; ++i) {
        QString sec = QString::number(i);
        filterSectionCombo->addItem(sec, sec);
    }

    QPushButton *filterBtn = new QPushButton("Apply Filter", studentsTab);
    connect(filterBtn, &QPushButton::clicked,
            this, &MainWindow::applyStudentFilter);

    QPushButton *autoSecBtn = new QPushButton("Auto Assign Sections", studentsTab);
    connect(autoSecBtn, &QPushButton::clicked,
            this, &MainWindow::autoAssignSections);

    QFormLayout *filterLayout = new QFormLayout;
    filterLayout->addRow("Year filter:",    filterYearCombo);
    filterLayout->addRow("Section filter:", filterSectionCombo);
    filterLayout->addRow(filterBtn);
    filterLayout->addRow(autoSecBtn);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addLayout(formLayout);
    rightLayout->addSpacing(20);
    rightLayout->addLayout(filterLayout);
    rightLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(studentsView, 3);
    mainLayout->addLayout(rightLayout, 1);

    studentsTab->setLayout(mainLayout);

    tabWidget->addTab(studentsTab, "Students");
}

// ============= Students slots =============
void MainWindow::addStudent()
{
    QString name = studentNameEdit->text().trimmed();
    QString sid  = studentIdEdit->text().trimmed();
    int year     = studentYearSpin->value();
    QString sec  = studentSectionCombo->currentText();

    if (name.isEmpty() || sid.isEmpty()) {
        QMessageBox::warning(this, "Input Error",
                             "Name and Student ID are required.");
        return;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        INSERT INTO Students (student_id, name, year, section)
        VALUES (:sid, :name, :year, :sec)
    )");
    q.bindValue(":sid",  sid);
    q.bindValue(":name", name);
    q.bindValue(":year", year);
    q.bindValue(":sec",  sec);

    if (!q.exec()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to add student: " + q.lastError().text());
        return;
    }

    studentsModel->select();
    refreshStudentCombo(); // علشان تبويب الدرجات
    studentNameEdit->clear();
    studentIdEdit->clear();
}

void MainWindow::applyStudentFilter()
{
    QString filter;

    int yearData = filterYearCombo->currentData().toInt();
    QString sec  = filterSectionCombo->currentData().toString();

    if (yearData != 0) {
        filter += QString("year = %1").arg(yearData);
    }

    if (!sec.isEmpty()) {
        if (!filter.isEmpty()) filter += " AND ";
        filter += QString("section = '%1'").arg(sec);
    }

    studentsModel->setFilter(filter);
    studentsModel->select();
}

void MainWindow::autoAssignSections()
{
    int yearData = filterYearCombo->currentData().toInt();
    if (yearData == 0) {
        QMessageBox::information(this, "Info",
                                 "Select a specific year from the Year filter first.");
        return;
    }

    QStringList sections;
    for (int i = 1; i <= 10; ++i)
        sections << QString::number(i);
    int idx = 0;

    QSqlQuery selectQ(db);
    selectQ.prepare("SELECT id FROM Students WHERE year = :y ORDER BY id");
    selectQ.bindValue(":y", yearData);

    if (!selectQ.exec()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to load students: " + selectQ.lastError().text());
        return;
    }

    QList<int> ids;
    while (selectQ.next()) {
        ids << selectQ.value(0).toInt();
    }

    QSqlQuery updateQ(db);
    for (int id : ids) {
        QString sec = sections.at(idx % sections.size());
        idx++;

        updateQ.prepare("UPDATE Students SET section = :s WHERE id = :id");
        updateQ.bindValue(":s", sec);
        updateQ.bindValue(":id", id);

        if (!updateQ.exec()) {
            QMessageBox::warning(this, "DB Error",
                                 "Failed to update student: " + updateQ.lastError().text());
            return;
        }
    }

    studentsModel->select();
    QMessageBox::information(this, "Done",
                             QString("Sections auto-assigned for year %1.").arg(yearData));
    refreshStudentCombo();
}

void MainWindow::deleteStudent()
{
    QModelIndex index = studentsView->currentIndex();

    if (!index.isValid()) {
        QMessageBox::warning(this, "Delete Student",
                             "Please select a student row first.");
        return;
    }

    int row = index.row();

    // عمود 0 في Students هو الـ id (PRIMARY KEY)
    int studentDbId = studentsModel->data(studentsModel->index(row, 0)).toInt();

    // تأكيد من المستخدم
    auto reply = QMessageBox::question(
        this,
        "Confirm delete",
        "Delete this student and all their grades?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
        return;

    // 1) نمسح كل الدرجات الخاصة بالطالب من جدول Grades
    {
        QSqlQuery q(db);
        q.prepare("DELETE FROM Grades WHERE student_id = :sid");
        q.bindValue(":sid", studentDbId);

        if (!q.exec()) {
            QMessageBox::critical(this, "DB Error",
                                  "Failed to delete student's grades: "
                                      + q.lastError().text());
            return;
        }
    }

    // 2) نمسح الطالب نفسه من جدول Students
    {
        QSqlQuery q(db);
        q.prepare("DELETE FROM Students WHERE id = :id");
        q.bindValue(":id", studentDbId);

        if (!q.exec()) {
            QMessageBox::critical(this, "DB Error",
                                  "Failed to delete student: "
                                      + q.lastError().text());
            return;
        }
    }

    // نعمل refresh للجداول والـ combos
    studentsModel->select();
    gradesModel->select();
    refreshStudentCombo();

    QMessageBox::information(this, "Deleted", "Student deleted successfully.");
}

// ================== Courses Tab ==================
void MainWindow::setupCoursesTab()
{
    coursesTab = new QWidget(this);

    coursesView  = new QTableView(coursesTab);
    coursesModel = new QSqlTableModel(coursesTab, db);
    coursesModel->setTable("Courses");
    coursesModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    coursesModel->select();

    coursesView->setModel(coursesModel);
    coursesView->horizontalHeader()->setStretchLastSection(true);
    coursesView->setSelectionBehavior(QAbstractItemView::SelectRows);
    coursesView->setColumnHidden(0, true);


    courseNameEdit    = new QLineEdit(coursesTab);
    courseCodeEdit    = new QLineEdit(coursesTab);
    courseCreditsSpin = new QSpinBox(coursesTab);
    courseCreditsSpin->setRange(1, 30);
    courseYearSpin    = new QSpinBox(coursesTab);
    courseYearSpin->setRange(1, 4);
    coursePrereqEdit  = new QLineEdit(coursesTab);

    QPushButton *addCourseBtn = new QPushButton("Add Course", coursesTab);
    connect(addCourseBtn, &QPushButton::clicked,
            this, &MainWindow::addCourse);
    QPushButton *deleteCourseBtn = new QPushButton("Delete Course", coursesTab);
    connect(deleteCourseBtn, &QPushButton::clicked,
            this, &MainWindow::deleteCourse);


    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Name:",         courseNameEdit);
    formLayout->addRow("Code:",         courseCodeEdit);
    formLayout->addRow("Credits:",      courseCreditsSpin);
    formLayout->addRow("Year (1-4):",   courseYearSpin);
    formLayout->addRow("Prerequisites:", coursePrereqEdit);
    formLayout->addRow(addCourseBtn);
    formLayout->addRow(deleteCourseBtn);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(coursesView, 3);
    mainLayout->addLayout(formLayout, 1);

    coursesTab->setLayout(mainLayout);
    tabWidget->addTab(coursesTab, "Courses");
}

void MainWindow::addCourse()
{
    QString name  = courseNameEdit->text().trimmed();
    QString code  = courseCodeEdit->text().trimmed();
    int credits   = courseCreditsSpin->value();
    int year      = courseYearSpin->value();
    QString prereq = coursePrereqEdit->text().trimmed();

    if (name.isEmpty() || code.isEmpty()) {
        QMessageBox::warning(this, "Input Error",
                             "Course name and code are required.");
        return;
    }

    QSqlQuery q(db);
    q.prepare(R"(
        INSERT INTO Courses (code, name, credits, year, prerequisites)
        VALUES (:code, :name, :cr, :year, :pre)
    )");
    q.bindValue(":code", code);
    q.bindValue(":name", name);
    q.bindValue(":cr", credits);
    q.bindValue(":year", year);
    q.bindValue(":pre", prereq);

    if (!q.exec()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to add course: " + q.lastError().text());
        return;
    }

    coursesModel->select();
    refreshCourseCombo();
    courseNameEdit->clear();
    courseCodeEdit->clear();
    coursePrereqEdit->clear();
}


void MainWindow::deleteCourse()
{
    // نجيب الصف المختار من جدول الكورسات
    QModelIndex index = coursesView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Delete Course",
                             "Please select a course row first.");
        return;
    }

    int row = index.row();

    // عمود 0 في Courses هو id (PRIMARY KEY)
    int courseDbId = coursesModel->data(coursesModel->index(row, 0)).toInt();

    // تأكيد من المستخدم
    auto reply = QMessageBox::question(
        this,
        "Confirm delete",
        "Delete this course and all related grades?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
        return;

    // 1) امسح كل الدرجات الخاصة بالكورس من جدول Grades
    {
        QSqlQuery q(db);
        q.prepare("DELETE FROM Grades WHERE course_id = :cid");
        q.bindValue(":cid", courseDbId);

        if (!q.exec()) {
            QMessageBox::critical(this, "DB Error",
                                  "Failed to delete course grades: "
                                      + q.lastError().text());
            return;
        }
    }

    // 2) امسح الكورس نفسه من جدول Courses
    {
        QSqlQuery q(db);
        q.prepare("DELETE FROM Courses WHERE id = :id");
        q.bindValue(":id", courseDbId);

        if (!q.exec()) {
            QMessageBox::critical(this, "DB Error",
                                  "Failed to delete course: "
                                      + q.lastError().text());
            return;
        }
    }

    // نحدّث الجداول والـ combo
    coursesModel->select();
    gradesModel->select();
    refreshCourseCombo();

    QMessageBox::information(this, "Deleted",
                             "Course deleted successfully.");
}


// ================== setupProfessorsTab ==================
void MainWindow::setupProfessorsTab()
{
    // منع التكرار
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i) == "Professors")
            return;
    }

    professorsTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(professorsTab);

    QLabel* title = new QLabel("Professors Management (Admin)");
    title->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(title);

    // ===== Model =====
    professorsModel = new QSqlTableModel(this, db);
    professorsModel->setTable("Professors");
    professorsModel->select();

    // ===== View =====
    professorsView = new QTableView(professorsTab);
    professorsView->setModel(professorsModel);
    professorsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    professorsView->setSelectionMode(QAbstractItemView::SingleSelection);

    mainLayout->addWidget(professorsView);

    // ===== Assigned Courses View =====
    QLabel* coursesLbl = new QLabel("Assigned Courses");
    coursesLbl->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(coursesLbl);

    QTableView* assignedCoursesView = new QTableView();
    mainLayout->addWidget(assignedCoursesView);

    // Model
    QSqlQueryModel* assignedCoursesModel = new QSqlQueryModel(this);
    assignedCoursesView->setModel(assignedCoursesModel);
    assignedCoursesView->setSelectionBehavior(QAbstractItemView::SelectRows);
    assignedCoursesView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto loadProfessorCourses = [=]() {

        QModelIndex index = professorsView->currentIndex();
        if (!index.isValid())
            return;

        int row = index.row();
        int professorDbId =
            professorsModel->record(row).value("id").toInt();

        QString query = R"(
        SELECT Courses.name
        FROM CourseAssignments
        JOIN Courses ON Courses.id = CourseAssignments.course_id
        WHERE CourseAssignments.professor_id = %1
    )";

        assignedCoursesModel->setQuery(
            query.arg(professorDbId), db
        );
        };


    // ===== Form =====
    QFormLayout* form = new QFormLayout();

    professorCodeEdit = new QLineEdit();
    professorNameEdit = new QLineEdit();
    professorEmailEdit = new QLineEdit();
    professorDeptEdit = new QLineEdit();

    form->addRow("Code:", professorCodeEdit);
    form->addRow("Name:", professorNameEdit);
    form->addRow("Email:", professorEmailEdit);
    form->addRow("Department:", professorDeptEdit);

    mainLayout->addLayout(form);

    // ===== Buttons =====
    QPushButton* addBtn = new QPushButton("Add Professor");
    QPushButton* delBtn = new QPushButton("Delete Professor");

    mainLayout->addWidget(addBtn);
    mainLayout->addWidget(delBtn);

    connect(addBtn, &QPushButton::clicked, this, [=]() {

        QSqlRecord rec = professorsModel->record();

        rec.setValue("professor_id", professorCodeEdit->text());
        rec.setValue("name", professorNameEdit->text());
        rec.setValue("email", professorEmailEdit->text());
        rec.setValue("department", professorDeptEdit->text());

        if (!professorsModel->insertRecord(-1, rec)) {
            QMessageBox::warning(this, "Error", "Failed to insert professor");
            return;
        }

        professorsModel->submitAll();
        professorsModel->select();  

        //  تفريغ الحقول
        professorCodeEdit->clear();
        professorNameEdit->clear();
        professorEmailEdit->clear();
        professorDeptEdit->clear();
        });

    connect(professorsView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        [=]() {
            loadProfessorCourses();
        });

    connect(delBtn, &QPushButton::clicked, this, [=]() {

        QModelIndex index = professorsView->currentIndex();

        if (!index.isValid()) {
            QMessageBox::warning(this, "Delete Professor",
                "Please select a professor first.");
            return;
        }

        int row = index.row();

        professorsModel->removeRow(row);

        if (!professorsModel->submitAll()) {
            QMessageBox::critical(this, "Error",
                "Failed to delete professor.");
            professorsModel->revertAll();
            return;
        }

        professorsModel->select();   
        });

    QLabel* assignLbl = new QLabel("Assign Course to Professor");
    assignLbl->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(assignLbl);

    QComboBox* courseCombo = new QComboBox();
    mainLayout->addWidget(courseCombo);

    // تحميل الكورسات
    QSqlQuery qc(db);
    qc.exec("SELECT id, name FROM Courses");
    while (qc.next()) {
        courseCombo->addItem(
            qc.value("name").toString(),
            qc.value("id")   // course_id
        );
    }

    QPushButton* assignBtn = new QPushButton("Assign Course");
    mainLayout->addWidget(assignBtn);


    connect(assignBtn, &QPushButton::clicked, this, [=]() {

        QModelIndex index = professorsView->currentIndex();
        if (!index.isValid()) {
            QMessageBox::warning(this, "Assign Course",
                "Please select a professor first.");
            return;
        }

        int row = index.row();

        int professorDbId =
            professorsModel->record(row).value("id").toInt();

        int courseId = courseCombo->currentData().toInt();

        QSqlQuery q(db);
        q.prepare(
            "INSERT OR IGNORE INTO CourseAssignments "
            "(professor_id, course_id) "
            "VALUES (:pid, :cid)"
        );
        q.bindValue(":pid", professorDbId);
        q.bindValue(":cid", courseId);

        if (!q.exec()) {
            QMessageBox::critical(this, "DB Error",
                q.lastError().text());
            return;
        }

        QMessageBox::information(this, "Success",
            "Course assigned successfully.");
        });

    loadProfessorCourses();

    tabWidget->addTab(professorsTab, "Professors");
}




// ================== Grades Tab ==================
void MainWindow::setupGradesTab()
{
    gradesTab = new QWidget(this);

    QSqlRelationalTableModel *relModel = new QSqlRelationalTableModel(this, db);
    gradesModel = relModel;

    relModel->setTable("Grades");

    relModel->setRelation(
        relModel->fieldIndex("student_id"),
        QSqlRelation("Students", "id", "student_id")
        );

    relModel->setRelation(
        relModel->fieldIndex("course_id"),
        QSqlRelation("Courses", "id", "code")
        );

    relModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    relModel->select();

    gradesView = new QTableView(gradesTab);
    gradesView->setModel(relModel);
    gradesView->horizontalHeader()->setStretchLastSection(true);
    gradesView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // اخفاء عمود  id لأنه مالوش لازمة للمستخدم
    gradesView->setColumnHidden(relModel->fieldIndex("id"), true);

    // =====================================================
    //   جزء إدخال الدرجات
    // =====================================================

    gradeStudentCombo = new QComboBox(gradesTab);
    gradeCourseCombo  = new QComboBox(gradesTab);

    gradeA1Combo = new QComboBox(gradesTab);
    gradeA2Combo = new QComboBox(gradesTab);

    gradeFinalSpin = new QSpinBox(gradesTab);
    gradeFinalSpin->setRange(0, 60);

    QStringList gradeOptions = {"NA", "P", "M", "D"};
    gradeA1Combo->addItems(gradeOptions);
    gradeA2Combo->addItems(gradeOptions);

    QPushButton *saveBtn = new QPushButton("Save Grade", gradesTab);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveGrade);

    QPushButton *deleteGradeBtn = new QPushButton("Delete Grade", gradesTab);
    connect(deleteGradeBtn, &QPushButton::clicked, this, &MainWindow::deleteGrade);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Student:", gradeStudentCombo);
    formLayout->addRow("Course:",  gradeCourseCombo);
    formLayout->addRow("Assignment 1:", gradeA1Combo);
    formLayout->addRow("Assignment 2:", gradeA2Combo);
    formLayout->addRow("Final (0–60):", gradeFinalSpin);
    formLayout->addRow(saveBtn);
    formLayout->addRow(deleteGradeBtn);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(gradesView, 3);
    mainLayout->addLayout(formLayout, 1);

    gradesTab->setLayout(mainLayout);
    tabWidget->addTab(gradesTab, "Grades");

    refreshStudentCombo();
    refreshCourseCombo();
}

// ============= Grades helpers =============
void MainWindow::refreshStudentCombo()
{
    if (!gradeStudentCombo) return;

    gradeStudentCombo->clear();

    QSqlQuery q(db);
    if (!q.exec("SELECT id, student_id, name FROM Students ORDER BY student_id")) {
        return;
    }

    while (q.next()) {
        int id = q.value(0).toInt();
        QString sid = q.value(1).toString();
        QString name = q.value(2).toString();
        QString text = sid + " - " + name;
        gradeStudentCombo->addItem(text, id);    }
}

void MainWindow::refreshCourseCombo()
{
    if (!gradeCourseCombo) return;

    gradeCourseCombo->clear();

    QSqlQuery q(db);
    if (!q.exec("SELECT id, code, name FROM Courses ORDER BY code")) {
        return;
    }

    while (q.next()) {
        int id = q.value(0).toInt();
        QString code = q.value(1).toString();
        QString name = q.value(2).toString();
        QString text = code + " - " + name;
        gradeCourseCombo->addItem(text, id);
    }
}

void MainWindow::saveGrade()
{
    if (!gradeStudentCombo || !gradeCourseCombo) return;

    int studentDbId = gradeStudentCombo->currentData().toInt();
    int courseDbId  = gradeCourseCombo->currentData().toInt();

    if (studentDbId == 0 || courseDbId == 0) {
        QMessageBox::warning(this, "Input Error",
                             "Please select a student and a course.");
        return;
    }

    QString a1 = gradeA1Combo->currentText();
    QString a2 = gradeA2Combo->currentText();
    int final = gradeFinalSpin->value();

    int rawTotal   = calculateTotal(a1, a2, QString::number(final));
    int finalTotal = applyMercy(rawTotal);

    // upsert (update لو موجود، insert لو مش موجود)
    QSqlQuery checkQ(db);
    checkQ.prepare("SELECT id FROM Grades WHERE student_id = :sid AND course_id = :cid");
    checkQ.bindValue(":sid", studentDbId);
    checkQ.bindValue(":cid", courseDbId);

    if (!checkQ.exec()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to check grade: " + checkQ.lastError().text());
        return;
    }

    if (checkQ.next()) {
        int id = checkQ.value(0).toInt();
        QSqlQuery upd(db);
        upd.prepare(R"(
            UPDATE Grades
            SET assignment1 = :a1,
                assignment2 = :a2,
                final_exam   = :fe,
                total_score  = :ts
            WHERE id = :id
        )");
        upd.bindValue(":a1", a1);
        upd.bindValue(":a2", a2);
        upd.bindValue(":fe", final);
        upd.bindValue(":ts", finalTotal);
        upd.bindValue(":id", id);
        if (!upd.exec()) {
            QMessageBox::critical(this, "DB Error",
                                  "Failed to update grade: " + upd.lastError().text());
            return;
        }
    } else {
        QSqlQuery ins(db);
        ins.prepare(R"(
            INSERT INTO Grades (student_id, course_id, assignment1, assignment2, final_exam, total_score)
            VALUES (:sid, :cid, :a1, :a2, :fe, :ts)
        )");
        ins.bindValue(":sid", studentDbId);
        ins.bindValue(":cid", courseDbId);
        ins.bindValue(":a1", a1);
        ins.bindValue(":a2", a2);
        ins.bindValue(":fe", final);
        ins.bindValue(":ts", finalTotal);

        if (!ins.exec()) {
            QMessageBox::critical(this, "DB Error",
                                  "Failed to insert grade: " + ins.lastError().text());
            return;
        }
    }

    gradesModel->select();
    QMessageBox::information(this, "Saved",
                             "Grade saved successfully.\nTotal (after mercy): "
                                 + QString::number(finalTotal));
}
void MainWindow::deleteGrade()
{
    QModelIndex index = gradesView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Delete Grade",
                             "Please select a grade row first.");
        return;
    }

    int row = index.row();
    int gradeId = gradesModel->data(gradesModel->index(row, 0)).toInt();

    auto reply = QMessageBox::question(
        this,
        "Confirm delete",
        "Delete this grade record?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
        return;

    QSqlQuery q(db);
    q.prepare("DELETE FROM Grades WHERE id = :id");
    q.bindValue(":id", gradeId);

    if (!q.exec()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to delete grade: "
                                  + q.lastError().text());
        return;
    }

    gradesModel->select();
    QMessageBox::information(this, "Deleted", "Grade deleted successfully.");
}
