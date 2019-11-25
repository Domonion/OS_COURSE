#include <string>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <wait.h>
#include <stdlib.h>
#include <algorithm>
#include <map>

using std::map;
using std::cin;
using std::string;
using std::vector;
using std::endl;
using std::cout;
using std::find_if;

static const auto lambda = [](int ch) {
    return !isspace(ch);
};

struct Enviroment {
    map<string, string> vars;

    explicit Enviroment(char *env[]) {
        for (int i = 0; env[i] != nullptr; i++) {
            string var(env[i]);
            int border = var.find('=');
            vars[var.substr(0, border)] = var.substr(border + 1);
        }
    }

    char **get_env() {
        char **res = new char *[vars.size() + 1];
        int j = 0;
        for (auto &i : vars) {
            res[j] = new char[i.first.size() + i.second.size() + 2];
            strcpy(res[j], (i.first + "=" + i.second).c_str());
            res[j][i.first.size() + i.second.size() + 1] = '\0';
            j++;
        }
        res[vars.size()] = nullptr;
        return res;
    }

    void add_var(vector<string> input) {
        for (size_t i = 1; i < input.size(); i++) {
            string var = input[i];
            int border = var.find('=');
            if (border == string::npos) {
                vars[var.substr(0, border)] = "";
            } else {
                vars[var.substr(0, border)] = var.substr(border + 1);
            }
        }
    }

    void remove_var(vector<string> input) {
        for (size_t i = 1; i < input.size(); i++) {
            string var = input[i];
            vars.erase(var);
        }
    }

};

void print_error(const string &msg) {
    int error = errno;
    std::cout << msg << endl
              << strerror(error) << endl;
}

vector<string> parse_variables(string input) {
    vector<string> res;
    size_t last = 0, now;
    input += ' ';
    while (last < input.size() &&
           (now = (find_if(input.begin() + last, input.end(), lambda) - input.begin())) != input.size()) {
        last = now;
        now = find_if_not(input.begin() + now, input.end(), lambda) - input.begin();
        res.emplace_back(input.substr(last, now - last));
        last = now;
    }
    return res;
}

void execute(vector<string> const &variables, Enviroment &enviroment) {
    char **newargv = new char *[variables.size() + 1];
    for (size_t i = 0; i < variables.size(); i++) {
        newargv[i] = new char[variables[i].size() + 1];
        strcpy(newargv[i], variables[i].c_str());
    }
    newargv[variables.size()] = nullptr;
    char **pupa = enviroment.get_env();
    execve(newargv[0], newargv, pupa);//if execve returns, then execve failed and it has -1 return code (man execve)
    print_error("execve failed");
    for (size_t i = 0; pupa[i] != nullptr; i++)
        delete[] pupa[i];
    delete[] pupa;
    for (size_t i = 0; i < variables.size(); i++)
        delete[] newargv[i];
    delete[] newargv;
}

void trim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), lambda));
    s.erase(find_if(s.rbegin(), s.rend(), lambda).base(), s.end());
}

int main(int argc, char **argv, char **env) {
    Enviroment enviroment(env);
    do {
        if(cin.eof())
            break;
        int return_code = 0;
        string input;
        cout << ">> ";
        cout.flush();
        getline(cin, input);
        trim(input);
        if (!input.empty()) {
            auto args = parse_variables(input);
            if (args[0] == "exit")
                return 0;
            if (args[0] == "add") {
                enviroment.add_var(args);
            } else if (args[0] == "remove") {
                enviroment.remove_var(args);
            } else {
                pid_t id = fork();
                if (id == -1) {
                    print_error("fork failed");
                } else if (id != 0) {
                    if (wait(&return_code) == -1) {
                        print_error("wait failed");
                    } else {
                        cout << "return code: " << return_code << endl;
                    }
                } else {
                    execute(args, enviroment);
                    return 0;
                }
            }
        }
    } while (true);
}
