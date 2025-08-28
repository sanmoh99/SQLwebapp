#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <deque>
#include <assert.h>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <stdio.h>
#include <string.h>
#include <utility>
#include <math.h>
#include <bitset>
#include <iomanip>
#include <complex>
#include <sstream>
#include <cctype>
#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

using namespace std;
//#define int long long
typedef pair<int,int> pii;
typedef vector<int> vi;
typedef long double ld;
typedef long long ll;
#define X first
#define Y second
#define all(o) o.begin(), o.end()
#define endl '\n'
#define IOS ios::sync_with_stdio(0), cin.tie(0)

string mapToString(const map<string,string>& mp) {
    ostringstream out;
    out << "[";
    bool first = true;
    for (const auto& kv : mp) {
        if (!first) out << ", ";
        out << "{"
            << kv.first << ": " << kv.second
            << "}";
        first = false;
    }
    out << "]";
    return out.str();
}

vector<string> split_tab(const string& s) {
    vector<string> out; string cur;
    for (char c : s) {
        if (c == '\t') { out.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    out.push_back(cur);
    return out;
}

vector<map<string,string>> get_map(const string& filename = "data.tsv") {
    ifstream fin(filename);
    if (!fin) {
        cerr << "could not open file: " << filename << "\n";
        return {}; // return empty table on error
    }

    string header;
    if (!getline(fin, header)) {
        cerr << "empty file\n";
        return {};
    }
    vector<string> cols = split_tab(header);

    vector<map<string,string>> table;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        auto parts = split_tab(line);
        map<string,string> rowMap;
        for (int i = 0; i < cols.size(); ++i)
            rowMap[cols[i]] = parts[i];
        table.push_back(std::move(rowMap));
    }
    return table;
}

// --- NEW: parse TSV directly from a string (for web UI) ---
vector<map<string,string>> get_map_from_string(const string& tsv) {
    stringstream ss(tsv);
    string header;
    vector<map<string,string>> out;

    if (!getline(ss, header)) return out;
    vector<string> cols = split_tab(header);

    string line;
    while (getline(ss, line)) {
        if (line.empty()) continue;
        auto parts = split_tab(line);
        if (parts.size() != cols.size()) continue;
        map<string,string> row;
        for (int i = 0; i < cols.size(); ++i) row[cols[i]] = parts[i];
        out.push_back(std::move(row));
    }
    return out;
}

vector<map<string,string> > table;

bool parse_number(string x, double& out){
    if(x.empty()) return false;
    char* end=nullptr;
    out = strtod(x.c_str(), &end);
    return end && *end=='\0';
}

bool is_truthy(string v){
    double d;
    if(parse_number(v,d)) return d!=0.0;
    return !(v.empty() || v=="0");
}

int precedence(string op){
    if(op=="OR") return 1;
    if(op=="AND") return 2;
    if(op=="=" || op=="!=" || op=="<" || op==">") return 3;
    return 0;
}

bool is_operator(string t){
    return t=="=" || t=="!=" || t=="<" || t==">" || t=="AND" || t=="OR";
}

vector<string> tokenize(string s){
    vector<string> t; int i=0,n=s.size();
    auto skip=[&]{ while(i<n && isspace((unsigned char)s[i])) ++i; };
    auto isIdStart=[&](char c){ return isalpha((unsigned char)c) || c=='_'; };
    auto isId=[&](char c){ return isalnum((unsigned char)c) || c=='_'; };

    while(true){
        skip(); if(i>=n) break;
        char c=s[i];
        if(c=='!' && i+1<n && s[i+1]=='='){ t.push_back("!="); i+=2; continue; }
        if(c=='(' || c==')' || c=='=' || c=='<' || c=='>'){ t.push_back(string(1,c)); ++i; continue; }
        if(c=='\'' || c=='"'){
            char q=c; ++i; string out;
            while(i<n){
                char d=s[i++];
                if(d==q) break;
                if(d=='\\' && i<n){
                    char e=s[i++];
                    if(e=='\\' || e=='\'' || e=='"') out.push_back(e);
                    else if(e=='n') out.push_back('\n');
                    else if(e=='t') out.push_back('\t');
                    else out.push_back(e);
                } else out.push_back(d);
            }
            t.push_back(out);
            continue;
        }
        if(isdigit((unsigned char)c) || (c=='.' && i+1<n && isdigit((unsigned char)s[i+1]))){
            int j=i++;
            while(i<n && isdigit((unsigned char)s[i])) ++i;
            if(i<n && s[i]=='.'){ ++i; while(i<n && isdigit((unsigned char)s[i])) ++i; }
            t.push_back(s.substr(j,i-j));
            continue;
        }
        if(isIdStart(c)){
            int j=i++;
            while(i<n && isId(s[i])) ++i;
            string w = s.substr(j,i-j);
            string u=w; for(char& ch:u) ch = (char)toupper((unsigned char)ch);
            if(u=="AND" || u=="OR") t.push_back(u);
            else t.push_back(w);
            continue;
        }
        ++i; // skip unknown char
    }
    return t;
}

vector<string> infix_to_postfix(vector<string> ts){
    vector<string> out, st;
    for(const auto& x: ts){
        if(x=="("){
            st.push_back(x);
        } else if(x==")"){
            while(!st.empty() && st.back()!="("){ out.push_back(st.back()); st.pop_back(); }
            if(!st.empty() && st.back()=="(") st.pop_back();
        } else if(is_operator(x)){
            while(!st.empty() && st.back()!="(" && precedence(st.back())>=precedence(x)){
                out.push_back(st.back()); st.pop_back();
            }
            st.push_back(x);
        } else {
            out.push_back(x);
        }
    }
    while(!st.empty()){
        if(st.back()!="(" && st.back()!=")") out.push_back(st.back());
        st.pop_back();
    }
    return out;
}

bool evaluate_postfix(vector<string> rpn){
    vector<string> st;
    auto pop2 = [&]()->pair<string,string>{
        if(st.size()<2) return {"0","0"};
        string b=st.back(); st.pop_back();
        string a=st.back(); st.pop_back();
        return {a,b};
    };
    for(const auto& x: rpn){
        if(!is_operator(x)){
            st.push_back(x);
        }
        else if(x=="AND"){
            auto pr=pop2(); st.push_back(is_truthy(pr.first) && is_truthy(pr.second) ? "1":"0");
        } 
        else if(x=="OR"){
            auto pr=pop2(); st.push_back(is_truthy(pr.first) || is_truthy(pr.second) ? "1":"0");
        } 
        else if(x=="=" || x=="!="){
            auto pr=pop2(); double da=0,db=0; bool na=parse_number(pr.first,da), nb=parse_number(pr.second,db);
            bool res = (na&&nb) ? (da==db) : (pr.first==pr.second);
            st.push_back( (x=="=" ? res : !res) ? "1":"0" );
        } 
        else if(x=="<" || x==">"){
            auto pr=pop2(); double da=0,db=0; bool na=parse_number(pr.first,da), nb=parse_number(pr.second,db);
            bool res = (na&&nb) ? ((x=="<") ? (da<db) : (da>db)) : false;
            st.push_back(res ? "1":"0");
        }
    }
    if(st.empty()) 
        return false;
    return is_truthy(st.back());
}

bool eval_expression(string s){
    return evaluate_postfix(infix_to_postfix(tokenize(s)));
}

vector<string> query(string s){
    vector<string> words;
    string cur = "";
    vector<string> ans;
    int where_idx = -1;
    int from_idx = -1;
    for(int i=0; i<s.size(); i++){
        if(s[i] == ' '){
            if(cur.size() > 0){
                if(cur == "WHERE")
                    where_idx = words.size();
                if(cur == "FROM")
                    from_idx = words.size();
                words.push_back(cur);
                cur = "";
            }
            continue;
        } else{
            cur += s[i];
        }
    }
    if(cur.size())
        words.push_back(cur);

    bool all_cols = false;
    vector<string> selected_cols;
    for(int i=1; i<from_idx; i++){
        if(words[i] == "*")
            all_cols = true;
        else
            selected_cols.push_back(words[i]);
    }
    if(all_cols == true && !table.empty()){
        for(const auto& kv : table[0]){
            selected_cols.push_back(kv.first);
        }
    }
    map<string, int> is_col;
    if(!table.empty()){
        for(const auto& kv : table[0]){
            is_col[kv.first] = 1;
        }
    }

    for(const auto& row : table){
        vector<string> tokens = words;
        for(int i=0; i<tokens.size(); i++){
            string x = tokens[i];
            if(is_col[x] == 1){
                auto it = row.find(x);
                if(it != row.end()) tokens[i] = it->second;
            }
        }
        bool after_where = false;
        string expr;
        for(const auto& token : tokens){
            if(after_where == true){
                expr += token;
                expr += ' ';
            }
            if(token == "WHERE")
                after_where = true;
        }
        bool is_good = expr.empty() ? true : eval_expression(expr);
        if(is_good){
            map<string, string> row_selected;
            for(const auto& col : selected_cols){
                auto it = row.find(col);
                if(it != row.end()) row_selected[col] = it->second;
            }
            ans.push_back(mapToString(row_selected));
        }
    }
    return ans;
}

// --- NEW: run multiple lines of queries and return one big string ---
string run_queries_text(const string& queries_text) {
    stringstream in(queries_text);
    string line;
    ostringstream out;
    bool firstOut = true;

    while (getline(in, line)) {
        if (line.empty()) continue;
        auto res = query(line);
        for (const auto& row : res) {
            if (!firstOut) out << "\n";
            out << row;
            firstOut = false;
        }
        if (!res.empty()) out << "\n"; // separate query blocks a bit
    }
    return out.str();
}

#ifdef __EMSCRIPTEN__
extern "C" {

EMSCRIPTEN_KEEPALIVE
void setTableFromTsv(const char* tsv_cstr) {
    table = get_map_from_string(tsv_cstr ? string(tsv_cstr) : string());
}

EMSCRIPTEN_KEEPALIVE
char* runQueries(const char* queries_cstr) {
    static string last;
    last = run_queries_text(queries_cstr ? string(queries_cstr) : string());
    return last.empty() ? (char*)"" : last.data();
}

} // extern "C"
#endif

#ifndef __EMSCRIPTEN__
// Native testing (optional)
int main(){
    IOS;
    cerr << "Native test mode. Reading data.tsv and then queries from stdin (EOF to end)." << endl;
    table = get_map("data.tsv");
    int n = 3; 
    string line; 
    for (int i = 0; i < n; i++){
        getline(cin, line); 
        auto res = query(line); 
        for (auto row : res){
            cout << row << endl; 
        } 
    }

}
#endif
