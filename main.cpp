#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <bitset>
#include <utility>
#include <functional>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
using namespace std;

struct Account {
    string user, pass, email, phone;
    int active; // 1: active, 0: blocked
    int role;   // 0: normal, 1: admin

    string toLine() const {
        return user+" "+pass+" "+email+" "+phone+" "+
               to_string(active)+" "+to_string(role);
    }

    static Account fromLine(const string& line) {
        Account a;
        stringstream ss(line);
        ss >> a.user >> a.pass >> a.email >> a.phone >> a.active >> a.role;
        return a;
    }
};

struct AccountManager {
    vector<Account> list;
    Account* cur = nullptr;

    void load(const string& fn="account.txt") {
        list.clear();
        ifstream f(fn);
        string s;
        while(getline(f,s)) if(!s.empty()) list.push_back(Account::fromLine(s));
    }

    void save(const string& fn="account.txt") {
        ofstream f(fn);
        for(auto& a:list) f<<a.toLine()<<"\n";
    }

    void log(const string& u) {
        ofstream f("history.txt",ios::app);
        time_t now=time(nullptr); char buf[32];
        strftime(buf,sizeof(buf),"%d/%m/%Y %H:%M:%S",localtime(&now));
        f<<u<<" | "<<buf<<"\n";
    }

    long lockTime(const string& u) {
        ifstream f("lock.txt"); string line;
        while(getline(f,line)){
            auto p=line.find('|'); if(p==string::npos) continue;
            if(line.substr(0,p)==u) return stol(line.substr(p+1));
        }
        return 0;
    }

    void setLock(const string& u,long t) {
        ifstream f("lock.txt"); vector<string> v; string line;
        while(getline(f,line)){
            auto p=line.find('|'); if(p==string::npos) continue;
            if(line.substr(0,p)!=u) v.push_back(line);
        }
        f.close();
        ofstream g("lock.txt");
        for(auto& s:v) g<<s<<"\n";
        if(t) g<<u<<"|"<<t<<"\n";
    }

    bool validEmail(const string& e){
        return e.size()>10 && e.rfind("@gmail.com")==e.size()-10;
    }
    bool validPhone(const string& p){
        return p.size()==10 && all_of(p.begin(),p.end(),::isdigit);
    }

    void signUp(){
        string u; cout<<"Username: "; getline(cin,u);
        if(u.empty()||any_of(list.begin(),list.end(),[&](auto& x){return x.user==u;})){
            cout<<"Username already taken or empty.\n"; return;
        }
        string p,e,t; 
        cout<<"Password: "; getline(cin,p);
        cout<<"Email: "; getline(cin,e);
        if(!validEmail(e)){cout<<"Invalid email.\n";return;}
        cout<<"Phone: "; getline(cin,t);
        if(!validPhone(t)){cout<<"Invalid phone.\n";return;}
        list.push_back({u,p,e,t,1,0});
        save(); cout<<"Sign up successful!\n";
    }

    void signIn(){
        string u; cout<<"Username: "; getline(cin,u);
        auto it=find_if(list.begin(),list.end(),[&](auto& x){return x.user==u;});
        if(it==list.end()){ cout<<"User not found.\n"; return; }

        if(it->active==0){
            long lockAt=lockTime(u), now=time(nullptr);
            if(lockAt && now-lockAt<600){ cout<<"Account locked. Try later.\n"; return;}
            it->active=1; setLock(u,0); save(); cout<<"Account unlocked automatically.\n";
        }

        for(int i=0;i<3;i++){
            string p; cout<<"Password: "; getline(cin,p);
            if(p==it->pass){ cout<<"Welcome, "<<u<<"!\n"; cur=&*it; log(u); return;}
            cout<<"Wrong password.\n";
        }
        it->active=0; setLock(u,time(nullptr)); save();
        cout<<"Too many wrong attempts. Account locked.\n";
    }

    void changePass(){
        if(!cur){ cout<<"Login first.\n"; return;}
        string o; cout<<"Old password: "; getline(cin,o);
        if(o!=cur->pass){cout<<"Incorrect.\n";return;}
        cout<<"New password: "; getline(cin,cur->pass);
        save(); cout<<"Password updated.\n";
    }

    void updateInfo(){
        if(!cur){ cout<<"Login first.\n"; return;}
        cout<<"1.Email  2.Phone: "; string c; getline(cin,c);
        if(c=="1"){ string e; cout<<"New email: "; getline(cin,e);
            if(validEmail(e)) cur->email=e; else {cout<<"Invalid email.\n";return;}
        } else if(c=="2"){ string t; cout<<"New phone: "; getline(cin,t);
            if(validPhone(t)) cur->phone=t; else {cout<<"Invalid phone.\n";return;}
        }
        save(); cout<<"Info updated.\n";
    }

    void resetPass(){
        string u; cout<<"Username: "; getline(cin,u);
        auto it=find_if(list.begin(),list.end(),[&](auto& x){return x.user==u;});
        if(it==list.end()){cout<<"No such user.\n";return;}
        int code=rand()%900000+100000;
        cout<<"(Verification code sent to "<<it->email<<": "<<code<<")\n";
        string in; cout<<"Enter code: "; getline(cin,in);
        if(stoi(in)!=code){ cout<<"Wrong code.\n";return;}
        cout<<"New password: "; getline(cin,it->pass);
        it->active=1; setLock(u,0); save(); cout<<"Password reset done.\n";
    }

    void viewHistory(){
        if(!cur){cout<<"Login first.\n";return;}
        ifstream f("history.txt"); string line;
        while(getline(f,line)) if(line.find(cur->user)==0) cout<<line<<"\n";
    }

    void signOut(){ if(cur){cout<<"Goodbye "<<cur->user<<"\n"; cur=nullptr;} }

    void adminMenu(){
        if(!cur||cur->role!=1){cout<<"Not authorized.\n";return;}
        cout<<"1.View all  2.Delete user  3.Reset user password\nChoice: ";
        string c; getline(cin,c);
        if(c=="1") for(auto& x:list) cout<<x.toLine()<<"\n";
        else if(c=="2"){ string u; cout<<"User to delete: "; getline(cin,u);
            auto it=remove_if(list.begin(),list.end(),[&](auto& x){return x.user==u;});
            if(it!=list.end()){ list.erase(it,list.end()); save(); cout<<"Deleted.\n"; }
        } else if(c=="3"){ string u; cout<<"User to reset: "; getline(cin,u);
            auto it=find_if(list.begin(),list.end(),[&](auto& x){return x.user==u;});
            if(it!=list.end()){ cout<<"New password: "; getline(cin,it->pass); it->active=1; save(); cout<<"Reset done.\n"; }
        }
    }
};

int main(){
    srand(time(nullptr));
    AccountManager m; m.load();
    while(true){
        cout<<"\n--- MENU ---\n"
              "1.Sign up\n2.Sign in\n3.Change password\n4.Update info\n"
              "5.Forgot password\n6.View history\n7.Sign out\n8.Admin\n0.Exit\nChoice: ";
        string c; getline(cin,c);
        if(c=="1") m.signUp();
        else if(c=="2") m.signIn();
        else if(c=="3") m.changePass();
        else if(c=="4") m.updateInfo();
        else if(c=="5") m.resetPass();
        else if(c=="6") m.viewHistory();
        else if(c=="7") m.signOut();
        else if(c=="8") m.adminMenu();
        else if(c=="0") break;
    }
}