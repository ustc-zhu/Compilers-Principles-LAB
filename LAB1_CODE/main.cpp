#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

string err_message[3] = {"",
                         "Array or Function cannot be returned from Functions",
                         "Array of Functions is not allowed"};
vector<string> decs;
vector<string> decs_no_syntax_error;
int s_index = 0;
int decs_cnt = 0;
string sym;
int err_pos = 0;
bool err_flag = false;  // syntax error

class ID {
 public:
  string idname;
  string type;
  string oritype;
  string para_type[30];
  int t_size = 1;
  string levstr[10];
  int level = -1;
  int maxlev = -1;
  vector<int> error_flag;               // semantic error;
  vector<vector<class ID>> parameters;  // for function
};

void split(const string &input) {
  string str;
  for (auto it = input.begin(); it != input.end(); it++) {
    str += *it;
    if (*it == ';' || it == input.end() - 1) {
      decs.push_back(str);
      str.erase();
    }
  }
}

void getsym() {
  sym = "";
  if (s_index != decs[decs_cnt].length()) {
    while (decs[decs_cnt][s_index] == ' ' || decs[decs_cnt][s_index] == '\t') {
      s_index++;
    }
    if (isalpha(decs[decs_cnt][s_index])) {
      while (isalpha(decs[decs_cnt][s_index])) {
        sym += decs[decs_cnt][s_index];
        s_index++;
      }
    } else if (isdigit(decs[decs_cnt][s_index])) {
      while (isdigit(decs[decs_cnt][s_index])) {
        sym += decs[decs_cnt][s_index];
        s_index++;
      }
    } else {
      sym += decs[decs_cnt][s_index];
      s_index++;
    }
  }
  // cout << sym << endl;
}

void pointer(ID &id) {
  id.levstr[id.level] += '*';
  getsym();
  while (sym == "*") {
    id.levstr[id.level] += '*';
    getsym();
  }
}
void error(int n) {
  if (n == 0)
    return;
  else if (n == -1) {
    cout << decs[decs_cnt] << endl;
    for (size_t i = 0; i != err_pos; i++) {
      cout << ' ';
    }
    cout << '^' << endl;
    cout << "error message: syntax error" << endl;
    cout << endl;
  } else {
    cout << "Type Checking......" << endl;
    cout << "error message: " << err_message[n] << endl;
    cout << endl;
  }
}

void PRINT(vector<string> decs, vector<vector<ID>> id_list) {
  for (auto it = id_list.begin(); it != id_list.end(); it++) {
    cout << decs[it - id_list.begin()] << endl;
    for (auto it2 = it->begin(); it2 != it->end(); it2++) {
      if (it2->type[0] == 'f') {  
        string return_type;
        auto it3 = it2->parameters.begin();
        if (it3 != it2->parameters.end()) {
          for (auto it4 = it3->begin(); it4 != it3->end(); it4++) {
            cout << "parameter: " << it4->idname
                 << " is type of : " << it4->type << endl;
          }
        }
        cout << it2->idname << " is the type of " << it2->type << endl;
        for (auto it3 = it2->error_flag.begin(); it3 != it2->error_flag.end();
             it3++) {
          error(*it3);
        }

        string temp = it2->type.substr(it2->type.find(it2->para_type[0]) +
                                       it2->para_type[0].length());
        size_t len = temp.length();
        for (size_t i = temp.find('>') + 1; i < len; i++) {
          return_type += temp[i];
        }
        return_type.pop_back();
        if (it2->error_flag.empty()) {
          cout << "Type Checking ...... OK!" << endl;
          cout << "Para-Type is : " << it2->para_type[0]
               << " , Return-Type is : " << return_type << endl;
        }
      } else if (it2->type[0] == 'a' || it2->type[0] == 'p') {
        string value_type;
        cout << it2->idname << " is the type of : " << it2->type << endl;
        size_t len = it2->type.length();
        if (it2->type[0] == 'p') {
          for (size_t i = it2->type.find("(") + 1; i < len; i++) {
            value_type += it2->type[i];
          }
        } else {
          for (size_t i = it2->type.find(",") + 1; i < len; i++) {
            value_type += it2->type[i];
          }
        }
        value_type.pop_back();
        for (auto it3 = it2->error_flag.begin(); it3 != it2->error_flag.end();
             it3++) {
          error(*it3);
        }
        if (it2->error_flag.empty()) {
          cout << "Type Checking ... OK!" << endl;
          cout << "Type Size : " << it2->t_size << endl;
          cout << "valuetype is : " << value_type << endl;
        }
      } else {
        cout << it2->idname << " is the type of " << it2->type << endl;
        for (auto it3 = it2->error_flag.begin(); it3 != it2->error_flag.end();
             it3++) {
          error(*it3);
        }
        if (it2->error_flag.empty()) {
          cout << "Type Checking ... OK!" << endl;
          cout << "Type Size : " << it2->t_size << endl;
        }
      }
    }
    cout << endl;
  }
}

void trans(vector<ID> &idtab) {
  // cout << idtab[0].levstr[1];
  int level, len, i, leftp, j, num, k;
  char buffer[10];
  auto it = idtab.begin();
  while (it != idtab.end()) {
    int func_cnt = 0, err_num = 0;
    int pre_flag = 0;  // 0:normal, 1: pointer, 2: array, 3: function
    level = it->maxlev;
    leftp = 0;
    while (level >= 0) {
      len = it->levstr[level].length();
      i = 0;
      while (it->levstr[level][i] != '[' && it->levstr[level][i] != '(' &&
             i < len) {
        i++;
      }
      while (i != len) {
        if (it->levstr[level][i] == '[') {
          if (pre_flag == 3) {
            it->type += '^';
            pre_flag = 2;
            it->error_flag.push_back(1);
          } else {
            pre_flag = 2;
          }
          it->type += "array(";
          leftp++;
          i++;
          k = 0;
          while (isdigit(it->levstr[level][i])) {
            it->type += it->levstr[level][i];
            buffer[k] = it->levstr[level][i];
            k++;
            i++;
          }
          // cout<<it->type<<endl;
          buffer[k] = '\0';
          num = atoi(buffer);
          if (level == it->maxlev) {
            it->t_size *= num;
          }
          it->type += ',';
          i++;
        }
        if (it->levstr[level][i] == '(') {
          if (pre_flag == 2) {
            it->type += '^';
            pre_flag = 3;
            it->error_flag.push_back(2);
          } else if (pre_flag == 3) {
            it->type += '^';
            pre_flag = 3;
            it->error_flag.push_back(1);
          } else {
            pre_flag = 3;
          }
          it->type += "function ( ";
          leftp++;
          for (auto it3 = it->parameters[func_cnt].begin();
               it3 != it->parameters[func_cnt].end(); it3++) {
            if (!it3->error_flag.empty()) {
              for (auto it4 = it3->error_flag.begin();
                   it4 != it3->error_flag.end(); it4++)
                it->error_flag.push_back(*it4);
            }
            it->para_type[func_cnt] += it3->type;
            if (it3 != it->parameters[func_cnt].end() - 1) {
              it->para_type[func_cnt] += " X";
            }
            it->para_type[func_cnt] += " ";
          }
          it->type += it->para_type[func_cnt] + " => ";
          func_cnt++;
          i += 2;  // skip'('
        }
      }
      i = 0;
      while (it->levstr[level][i] == '*' && i < len) {
        i++;
      }
      i--;
      while (i >= 0) {
        it->type += "pointer(";
        pre_flag = 1;
        leftp++;
        i--;
      }
      if (level == 0) {
        it->type += it->oritype;
        while (leftp > 0) {
          it->type += ')';
          leftp--;
        }
      }
      level--;
    }
    it++;
  }
}

void parameter_declaration(ID &id) {
  void declaration_specifiers(string & oritype);
  void init_declarator(ID & id);
  string oritype;
  declaration_specifiers(oritype);
  if (err_flag) return;
  id.oritype = oritype;
  init_declarator(id);
  if (err_flag) return;
}

void parameter_list(ID &id) {
  vector<ID> paras;
  if (sym != ")") {
    ID para;
    parameter_declaration(para);
    if (err_flag) return;
    paras.push_back(para);
  }
  while (sym == ",") {
    ID para;
    getsym();
    parameter_declaration(para);
    if (err_flag) return;
    paras.push_back(para);
  }
  trans(paras);
  id.parameters.push_back(paras);
  id.levstr[id.level] += sym;
  getsym();
}

void direct_declarator_1(ID &id) {
  if (sym == "[") {
    id.levstr[id.level] += sym;
    getsym();
    id.levstr[id.level] += sym;
    getsym();
    id.levstr[id.level] += sym;
    getsym();
    direct_declarator_1(id);
    if (err_flag) return;
  } else if (sym == "(") {
    id.levstr[id.level] += sym;
    getsym();
    // id.isfunc = 1;
    parameter_list(id);
    if (err_flag) return;
    direct_declarator_1(id);
    if (err_flag) return;
  } else {
    return;
  }
}
void direct_declarator(ID &id) {
  void init_declarator(ID & id);
  if (sym == "(") {
    // string levstr_n;
    getsym();
    init_declarator(id);
    if (err_flag) return;
    getsym();
    direct_declarator_1(id);
    if (err_flag) return;
  } else if (isalpha(sym[0])) {
    id.idname = sym;
    if (sym == "int" || sym == "void") {
      err_pos = s_index;
      error(-1);
      err_flag = true;
    }
    getsym();
    direct_declarator_1(id);
    if (err_flag) return;
  } else {
    err_pos = s_index;
    error(-1);
    err_flag = true;
  }
}
void init_declarator(ID &id) {
  id.level++;
  id.maxlev++;
  if (sym == "*") {
    pointer(id);
    if (err_flag) return;
    direct_declarator(id);
    if (err_flag) return;
  } else {
    direct_declarator(id);
    if (err_flag) return;
  }
  id.level--;
}
void init_declarator_list(vector<ID> &idtab) {
  class ID ID_1;
  init_declarator(ID_1);
  if (err_flag) return;
  idtab.push_back(ID_1);
  while (sym == ",") {
    class ID ID_1;
    getsym();
    init_declarator(ID_1);
    if (err_flag) return;
    idtab.push_back(ID_1);
  }
  return;
}
void declaration_specifiers(string &oritype) {
  if (sym == "int") {
    oritype = "int";
  } else if (sym == "void") {
    oritype = "void";
  } else {
    err_pos = s_index;
    error(-1);
    err_flag = true;
  }
  getsym();
}
void declaration(vector<ID> &idtab) {
  string oritype;
  declaration_specifiers(oritype);
  if (err_flag) return;
  init_declarator_list(idtab);
  if (err_flag) return;
  for (auto it = idtab.begin(); it != idtab.end(); it++) {
    it->oritype = oritype;
  }
  trans(idtab);
  if (sym == ";") {
    decs_no_syntax_error.push_back(decs[decs_cnt]);
    decs_cnt++;
    s_index = 0;
  } else {
    err_pos = s_index;
    error(-1);
    err_flag = true;
  }
}

void translation_unit(vector<vector<ID>> &id_list) {
  vector<ID> idtab;
  declaration(idtab);
  if (!err_flag) {
    id_list.push_back(idtab);
  } else {
    decs_cnt++;
    s_index = 0;
    err_flag = false;
    err_pos = 0;
  }
  while (decs.begin() + decs_cnt != decs.end()) {
    getsym();
    idtab.clear();
    declaration(idtab);
    if (!err_flag) {
      id_list.push_back(idtab);
    } else {
      decs_cnt++;
      s_index = 0;
      err_flag = false;
      err_pos = 0;
    }
  }
}

void start() {
  string s;
  vector<vector<ID>> id_list;
  cout << "please input some declarations" << endl;
  getline(cin, s);
  cout << endl;
  split(s);
  getsym();
  translation_unit(id_list);
  PRINT(decs_no_syntax_error, id_list);
}

int main() {
  start();
  return 0;
}
