# University Management System (C++ Desktop Application)

A desktop-based University Management System developed in **C++** using **Microsoft Visual Studio**, integrated with a local database to manage university administration workflows, user authentication, and role-based access control.

---

## 🚀 Key Features

- **Role-Based Access Control (RBAC):**
  - **Admin:** Full privileges to manage students, courses, grades, and system configurations.
  - **Professor / Faculty:** Dedicated interface (`professorwindow`) for grade submission and student monitoring.
  - **Student:** View-only access restricted strictly to individual grades, enrolled courses, and student records.
- **User Authentication:** Modular login window (`login.cpp` / `login.h`) validating credentials and dynamically managing permissions according to user role.
- **Relational Database Integration:** Local relational database (`university`) managing data persistence for students, faculty, and academic records.
- **Multilingual Support:** Localized resources including Arabic interface strings (`university_system_ar_EG.ts`).

---

## 🛠️ Tech Stack & Tools

- **Programming Language:** C++ (99.5%)
- **IDE:** Microsoft Visual Studio (MSVC)
- **GUI Framework:** Qt Widgets
- **Database:** SQLite / Local Relational DB

---

## 📂 Project Structure

```text
├── login.cpp / login.h             # Authentication window logic & header
├── mainwindow.cpp / mainwindow.h   # Main administration dashboard
├── mainwindow.ui                   # Qt Designer UI layout file
├── professorwindow.cpp / .h        # Faculty/Professor portal
├── university                      # Database storage file
├── university_system.vcxproj       # Visual Studio project file
├── university_system.pro           # QMake project file
└── university_system_ar_EG.ts      # Localization source file

⚙️ How to Build and Run
Using Microsoft Visual Studio:
Open Microsoft Visual Studio.

Select Open a project or solution.

Choose university_system.vcxproj.

Ensure Qt Visual Studio Tools extension / dependencies are configured if prompted.

Build and run the project (Ctrl + F5).

Using Qt Creator:
Open Qt Creator.

Select Open File or Project and open university_system.pro.

Configure your build kit (MSVC or MinGW).

Click Run (Ctrl + R).

👤 Author
Abdullah Alsaied

Developed as an academic software engineering project demonstrating Object-Oriented Programming (OOP), modular desktop GUI design, and database integration.
