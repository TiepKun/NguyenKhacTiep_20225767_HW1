#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
using namespace std;

/* ===== Class Account ===== */
class Account {
private:
    string username, password, email, phone;
    int status; // 1 active, 0 blocked
    int role;   // 0 user, 1 admin

public:
    Account(string u="", string p="", string e="", string ph="", int s=1, int r=0)
        : username(u), password(p), email(e), phone(ph), status(s), role(r) {}

    // getters
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getEmail() const { return email; }
    string getPhone() const { return phone; }
    int getStatus() const { return status; }
    int getRole() const { return role; }

    // setters
    void setPassword(const string& p) { password = p; }
    void setEmail(const string& e) { email = e; }
    void setPhone(const string& ph) { phone = ph; }
    void setStatus(int s) { status = s; }
    void setRole(int r) { role = r; }

    // IO helpers
    string toString() const {
        ostringstream oss;
        oss << username << " " << password << " "
            << email << " " << phone << " " << status << " " << role;
        return oss.str();
    }
    static Account fromString(const string& line) {
        istringstream iss(line);
        string u,p,e,ph; int s,r;
        iss >> u >> p >> e >> ph >> s >> r;
        return Account(u,p,e,ph,s,r);
    }
};

/* ===== Class AccountManager ===== */
class AccountManager {
private:
    vector<Account> accounts;
    Account* currentUser = nullptr;

    void saveAccounts(const string& filename) {
        ofstream fout(filename);
        for (auto& acc : accounts) fout << acc.toString() << "\n";
        fout.close();
    }
    void logLogin(const string& username) {
        ofstream fout("history.txt", ios::app);
        time_t now = time(nullptr);
        tm* t = localtime(&now);
        char buf[64];
        strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", t);
        fout << username << " | " << buf << "\n";
        fout.close();
    }

public:
    void loadAccounts(const string& filename) {
        accounts.clear();
        ifstream fin(filename);
        string line;
        while (getline(fin, line)) {
            if (!line.empty())
                accounts.push_back(Account::fromString(line));
        }
        fin.close();
    }

    void registerAccount() {
        string u,p,e,ph;
        cout << "Enter username: "; cin >> u;
        for (auto& acc : accounts) {
            if (acc.getUsername() == u) {
                cout << "Username already exists.\n"; return;
            }
        }
        cout << "Enter password: "; cin >> p;
        cout << "Enter email: "; cin >> e;
        cout << "Enter phone: "; cin >> ph;
        accounts.emplace_back(u,p,e,ph,1,0);
        saveAccounts("account.txt");
        cout << "Register success!\n";
    }

    void signIn() {
        string u,p;
        cout << "Enter username: "; cin >> u;
        for (auto& acc : accounts) {
            if (acc.getUsername() == u) {
                if (acc.getStatus() == 0) {
                    cout << "Your account is blocked.\n"; return;
                }
                int tries=0;
                while (tries<3) {
                    cout << "Enter password: "; cin >> p;
                    if (p == acc.getPassword()) {
                        cout << "Welcome " << u << "!\n";
                        currentUser = &acc;
                        logLogin(u);
                        return;
                    } else {
                        cout << "Wrong password.\n";
                        tries++;
                    }
                }
                acc.setStatus(0);
                saveAccounts("account.txt");
                cout << "Too many fails. Account blocked.\n";
                return;
            }
        }
        cout << "Account not found.\n";
    }

    void changePassword() {
        if (!currentUser) { cout << "Login first.\n"; return; }
        string oldp,newp;
        cout << "Old password: "; cin >> oldp;
        if (oldp != currentUser->getPassword()) {
            cout << "Wrong old password.\n"; return;
        }
        cout << "New password: "; cin >> newp;
        currentUser->setPassword(newp);
        saveAccounts("account.txt");
        cout << "Password changed.\n";
    }

    void updateAccountInfo() {
        if (!currentUser) { cout << "Login first.\n"; return; }
        int ch;
        cout << "1. Update email\n2. Update phone\nChoice: ";
        cin >> ch;
        if (ch==1) {
            string e; cout << "New email: "; cin >> e;
            currentUser->setEmail(e);
        } else if (ch==2) {
            string ph; cout << "New phone: "; cin >> ph;
            currentUser->setPhone(ph);
        }
        saveAccounts("account.txt");
        cout << "Info updated.\n";
    }

    void resetPassword() {
        string u; cout << "Username: "; cin >> u;
        for (auto& acc : accounts) {
            if (acc.getUsername() == u) {
                srand(time(nullptr));
                int code = rand()%900000+100000;
                cout << "[Simulated] Code sent to " << acc.getEmail()
                     << ": " << code << "\n";
                int input; cout << "Enter code: "; cin >> input;
                if (input != code) { cout << "Wrong code.\n"; return; }
                string newp; cout << "New password: "; cin >> newp;
                acc.setPassword(newp);
                acc.setStatus(1);
                saveAccounts("account.txt");
                cout << "Password reset success.\n";
                return;
            }
        }
        cout << "No such account.\n";
    }

    void viewLoginHistory() {
        if (!currentUser) { cout << "Login first.\n"; return; }
        ifstream fin("history.txt");
        string line;
        while (getline(fin, line)) {
            if (line.find(currentUser->getUsername()) == 0)
                cout << line << "\n";
        }
        fin.close();
    }

    void signOut() {
        if (!currentUser) { cout << "No user logged in.\n"; return; }
        cout << "Goodbye " << currentUser->getUsername() << "!\n";
        currentUser = nullptr;
    }
};

/* ===== Main ===== */
void menu() {
    cout << "\nUSER MANAGEMENT PROGRAM\n";
    cout << "-------------------------\n";
    cout << "1. Register\n";
    cout << "2. Sign in\n";
    cout << "3. Change password\n";
    cout << "4. Update account info\n";
    cout << "5. Reset password\n";
    cout << "6. View login history\n";
    cout << "7. Sign out\n";
    cout << "Other: Quit\n";
    cout << "Choice: ";
}

int main() {
    AccountManager manager;
    manager.loadAccounts("account.txt");

    int choice;
    while (true) {
        menu();
        if (!(cin >> choice)) break;
        switch(choice) {
            case 1: manager.registerAccount(); break;
            case 2: manager.signIn(); break;
            case 3: manager.changePassword(); break;
            case 4: manager.updateAccountInfo(); break;
            case 5: manager.resetPassword(); break;
            case 6: manager.viewLoginHistory(); break;
            case 7: manager.signOut(); break;
            default: cout << "Exit.\n"; return 0;
        }
    }
}
