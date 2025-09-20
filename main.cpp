#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cctype>
using namespace std;

const int UNLOCK_MINUTES = 10; // 10 phút auto-unlock

/* =================== Utilities =================== */
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

string safeInput(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    return trim(s);
}

bool isValidPhone(const string& ph) {
    if (ph.size() != 10) return false;
    for (char c : ph) if (!isdigit(c)) return false;
    return true;
}

bool isValidEmail(const string& e) {
    string suffix = "@gmail.com";
    if (e.size() <= suffix.size()) return false;
    return e.compare(e.size()-suffix.size(), suffix.size(), suffix) == 0;
}

/* =================== Account Class =================== */
class Account {
private:
    string username, password, email, phone;
    int status; // 1 active, 0 blocked
    int role;   // 0 user, 1 admin

public:
    Account(string u="", string p="", string e="", string ph="", int s=1, int r=0)
        : username(u), password(p), email(e), phone(ph), status(s), role(r) {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getEmail() const { return email; }
    string getPhone() const { return phone; }
    int getStatus() const { return status; }
    int getRole() const { return role; }

    void setPassword(const string& p) { password = p; }
    void setEmail(const string& e) { email = e; }
    void setPhone(const string& ph) { phone = ph; }
    void setStatus(int s) { status = s; }
    void setRole(int r) { role = r; }

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

/* =================== AccountManager =================== */
class AccountManager {
private:
    vector<Account> accounts;
    Account* currentUser = nullptr;

    void saveAccounts(const string& filename) {
        ofstream fout(filename);
        for (auto& acc : accounts) fout << acc.toString() << "\n";
    }
    void logLogin(const string& username) {
        ofstream fout("history.txt", ios::app);
        time_t now = time(nullptr);
        tm* t = localtime(&now);
        char buf[64];
        strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", t);
        fout << username << " | " << buf << "\n";
    }

    time_t getLockTime(const string& user) {
        ifstream fin("lock.txt");
        string line;
        while (getline(fin,line)) {
            string u; long long t;
            if (sscanf(line.c_str(), "%[^|]|%lld", &u[0], &t)) {}
        }
        fin.close();
        ifstream fin2("lock.txt");
        while (getline(fin2,line)) {
            size_t pos = line.find('|');
            if (pos!=string::npos) {
                string u = line.substr(0,pos);
                long long t = stoll(line.substr(pos+1));
                if (u==user) return (time_t)t;
            }
        }
        return 0;
    }
    void setLockTime(const string& user, time_t t) {
        ifstream fin("lock.txt");
        vector<string> lines;
        string line; bool found=false;
        while (getline(fin,line)) {
            size_t pos=line.find('|');
            if (pos!=string::npos) {
                string u=line.substr(0,pos);
                if (u==user) { found=true; continue; }
            }
            lines.push_back(line);
        }
        fin.close();
        ofstream fout("lock.txt");
        for (auto& l:lines) fout<<l<<"\n";
        if (t!=0) fout<<user<<"|"<<t<<"\n";
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
    }

    /* ==== Các chức năng ==== */
    void registerAccount() {
        string u = safeInput("Enter username: ");
        if (u.empty()) { cout<<"Username cannot be empty\n"; return; }
        for (auto& acc : accounts)
            if (acc.getUsername()==u) { cout<<"Username already exists.\n"; return; }
        string p = safeInput("Enter password: ");
        string e = safeInput("Enter email: ");
        if (!isValidEmail(e)) { cout<<"Invalid email (must end with @gmail.com)\n"; return; }
        string ph = safeInput("Enter phone: ");
        if (!isValidPhone(ph)) { cout<<"Phone must be 10 digits.\n"; return; }
        accounts.emplace_back(u,p,e,ph,1,0);
        saveAccounts("account.txt");
        cout<<"Register success!\n";
    }

    void signIn() {
        string u = safeInput("Enter username: ");
        for (auto& acc : accounts) {
            if (acc.getUsername()==u) {
                if (acc.getStatus()==0) {
                    time_t lockedAt=getLockTime(u);
                    time_t now=time(nullptr);
                    if (lockedAt!=0 && difftime(now,lockedAt)>=UNLOCK_MINUTES*60) {
                        cout<<"Auto-unlock after "<<UNLOCK_MINUTES<<" minutes.\n";
                        acc.setStatus(1);
                        setLockTime(u,0);
                        saveAccounts("account.txt");
                    } else {
                        cout<<"Your account is blocked. Try again later.\n"; return;
                    }
                }
                int tries=0; string p;
                while (tries<3) {
                    p = safeInput("Enter password: ");
                    if (p==acc.getPassword()) {
                        cout<<"Welcome "<<u<<"!\n";
                        currentUser=&acc;
                        logLogin(u);
                        return;
                    } else { cout<<"Wrong password.\n"; tries++; }
                }
                acc.setStatus(0);
                setLockTime(u,time(nullptr));
                saveAccounts("account.txt");
                cout<<"Too many fails. Account blocked.\n";
                return;
            }
        }
        cout<<"Account not found.\n";
    }

    void changePassword() {
        if (!currentUser) { cout<<"Login first.\n"; return; }
        string oldp = safeInput("Old password: ");
        if (oldp!=currentUser->getPassword()) { cout<<"Wrong old password.\n"; return; }
        string newp = safeInput("New password: ");
        currentUser->setPassword(newp);
        saveAccounts("account.txt");
        cout<<"Password changed.\n";
    }

    void updateAccountInfo() {
        if (!currentUser) { cout<<"Login first.\n"; return; }
        string ch = safeInput("1.Update email 2.Update phone: ");
        if (ch=="1") {
            string e = safeInput("New email: ");
            if (!isValidEmail(e)) { cout<<"Invalid email.\n"; return; }
            currentUser->setEmail(e);
        } else if (ch=="2") {
            string ph = safeInput("New phone: ");
            if (!isValidPhone(ph)) { cout<<"Invalid phone.\n"; return; }
            currentUser->setPhone(ph);
        }
        saveAccounts("account.txt");
        cout<<"Info updated.\n";
    }

    void resetPassword() {
        string u = safeInput("Username: ");
        for (auto& acc:accounts) {
            if (acc.getUsername()==u) {
                srand(time(nullptr));
                int code=rand()%900000+100000;
                cout<<"[Simulated] Code sent to "<<acc.getEmail()<<": "<<code<<"\n";
                string in = safeInput("Enter code: ");
                if (stoi(in)!=code) { cout<<"Wrong code.\n"; return; }
                string newp = safeInput("New password: ");
                acc.setPassword(newp); acc.setStatus(1);
                setLockTime(u,0);
                saveAccounts("account.txt");
                cout<<"Password reset success.\n"; return;
            }
        }
        cout<<"No such account.\n";
    }

    void viewLoginHistory() {
        if (!currentUser) { cout<<"Login first.\n"; return; }
        ifstream fin("history.txt");
        string line;
        while (getline(fin,line)) {
            if (line.find(currentUser->getUsername())==0) cout<<line<<"\n";
        }
    }

    void signOut() {
        if (!currentUser) { cout<<"No user logged in.\n"; return; }
        cout<<"Goodbye "<<currentUser->getUsername()<<"!\n";
        currentUser=nullptr;
    }

    /* ==== Admin functions ==== */
    void adminMenu() {
        if (!currentUser || currentUser->getRole()!=1) { cout<<"Not admin.\n"; return; }
        cout<<"--- Admin Menu ---\n";
        cout<<"1. View all accounts\n";
        cout<<"2. Delete account\n";
        cout<<"3. Reset password for user\n";
        string c = safeInput("Choice: ");
        if (c=="1") {
            for (auto& acc:accounts)
                cout<<acc.toString()<<"\n";
        } else if (c=="2") {
            string u = safeInput("Username to delete: ");
            for (auto it=accounts.begin(); it!=accounts.end(); ++it) {
                if (it->getUsername()==u) {
                    accounts.erase(it);
                    saveAccounts("account.txt");
                    cout<<"Deleted.\n"; return;
                }
            }
            cout<<"No such user.\n";
        } else if (c=="3") {
            string u = safeInput("Username to reset: ");
            for (auto& acc:accounts) {
                if (acc.getUsername()==u) {
                    string np = safeInput("New password: ");
                    acc.setPassword(np); acc.setStatus(1);
                    saveAccounts("account.txt");
                    cout<<"Password reset for "<<u<<"\n"; return;
                }
            }
            cout<<"No such user.\n";
        }
    }
};

/* =================== MAIN =================== */
void menu(bool isAdmin) {
    cout<<"\nUSER MANAGEMENT PROGRAM\n";
    cout<<"1. Register\n2. Sign in\n3. Change password\n4. Update account info\n";
    cout<<"5. Reset password\n6. View login history\n7. Sign out\n";
    if (isAdmin) cout<<"8. Admin menu\n";
    cout<<"Other: Quit\nChoice: ";
}

int main() {
    AccountManager manager;
    manager.loadAccounts("account.txt");
    while (true) {
        menu(true); // menu chung, Admin mới thấy mục 8
        string ch; getline(cin,ch); ch=trim(ch);
        if (ch=="1") manager.registerAccount();
        else if (ch=="2") manager.signIn();
        else if (ch=="3") manager.changePassword();
        else if (ch=="4") manager.updateAccountInfo();
        else if (ch=="5") manager.resetPassword();
        else if (ch=="6") manager.viewLoginHistory();
        else if (ch=="7") manager.signOut();
        else if (ch=="8") manager.adminMenu();
        else { cout<<"Exit.\n"; break; }
    }
}
