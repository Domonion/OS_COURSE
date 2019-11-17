#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <dirent.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdexcept>
#include <set>
#include <wait.h>
#include <stdlib.h>

using std::vector;
using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::runtime_error;
using std::set;
using std::cerr;
using std::stoi;
using std::to_string;

using inode_t = long;

const size_t KB = 1024;

void print_error(string const &msg) {
    int error = errno;
    if (string(strerror(error)) == "Permission denied")
        return;
    cerr << "Error: " + msg << '\n'
         << strerror(error) << '\n';
}

void print_usage() {
    cerr << "Usage: find PATH"
         << " [-inum NUMBER]"
         << " [-name FILENAME]"
         << " [-size [-=+]NUMBER]"
         << " [-nlinks NUMBER]"
         << " [-exec PATH]" << '\n'
         << " Example: ./find / -inum 1337 -name AAAAAA -size +100500 -nlinks 0 -exec /bin/rm" << '\n';
}

void end() {
    print_usage();
    exit(0);
}

bool check_error(int rc, string const &msg) {
    if (rc == -1) {
        print_error(msg);
        return false;
    }
    return true;
}

template<typename T>
bool check_ptr(T *ptr, const string &s = "", bool to_throw = false) {
    if (ptr == nullptr) {
        cerr << "Pointer is null, sth is totally wrong!" << '\n'
             << typeid(T).name() << '\n';
        if (to_throw)
            throw runtime_error("Expecting not nullptr, " + s);
        return false;
    }
    return true;
}

struct linux_dirent {
    inode_t d_ino; /* Inode */
    off_t d_off; /* I don'type know what is it */
    unsigned short d_reclen; /* length */
    char d_name[]; /*Filename (null-terminated)*/
};


bool getStat(string const &fp, struct stat *sb, linux_dirent *dirent) {
    return check_error(lstat(fp.c_str(), sb), "lstat failed with file " + string(dirent->d_name));
}

struct Base_validator {
protected:
    string fp;
public:

    bool check(linux_dirent *dirent) {
        if (check_ptr(dirent))
            return valid(dirent);
        return false;
    }

    virtual bool valid(linux_dirent *dirent) = 0;

    virtual ~Base_validator() = default;

    void set_fp(string const &_fp) {
        fp = _fp;
    }
};

struct Inum_validator : public Base_validator {
private:
    inode_t inode;
public:
    explicit Inum_validator(inode_t _inode) : inode(_inode) {}

    bool valid(linux_dirent *dirent) override {
        return dirent->d_ino == inode;
    }
};

struct Name_validator : Base_validator {
private:
    string filename;
public:
    explicit Name_validator(string _filename) : filename(std::move(_filename)) {}

    bool valid(linux_dirent *dirent) override {
        return filename == string(dirent->d_name);
    }
};

struct Size_validator : Base_validator {
private:
    off_t size;
    char type;
public:
    explicit Size_validator(off_t _size, char _type) : size(_size), type(_type) {
        //for debug, never throws(I hope so)
        if (type != '+' && type != '-' && type != '=')
            throw runtime_error("-size arg is wrong");
    }

    bool valid(linux_dirent *dirent) override {
        //off_t stat.si_size - size in bytes
        struct stat sb{};
        if (!getStat(fp, &sb, dirent)) {
            return false;
        }
        if (type == '-')
            return sb.st_size < size;
        else if (type == '=')
            return sb.st_size == size;
        return sb.st_size > size;
    }
};

struct NLLink_validator : Base_validator {
private:
    nlink_t nlink;
public:
    explicit NLLink_validator(nlink_t _nlink) : nlink(_nlink) {}

    bool valid(linux_dirent *dirent) override {
        struct stat sb{};
        if (!getStat(fp, &sb, dirent)) {
            return false;
        }
        return nlink == sb.st_nlink;
    }
};

const int KEY_N = 5;
const vector<string> KEYS{"-inum", "-name", "-size", "-nlinks", "-exec"};

//TODO symlinks and dirs!?

//name - any string
//exec - any string, if not exists - execve fails
//dir - any string, if not exists - will find nothing, open will fail
//inum. size, nlinks - stoi parses it.
struct Arguments {
private:
    string dir;

    vector<Base_validator *> validators;

    vector<string> exec_parametrs;
public:

    void set_validator(Base_validator *ptr) {
        if (check_ptr(ptr))
            validators.emplace_back(ptr);
    }

    void add_exec(string const &str) {
        exec_parametrs.emplace_back(str);
    }

    explicit Arguments(string _dir) : dir(std::move(_dir)) {
        if (dir.back() != '/')
            dir.push_back('/');
    }

    vector<string> const &get_exec() {
        return exec_parametrs;
    }

    string const &get_dir() {
        return dir;
    }

    vector<Base_validator *> const &get_validators() {
        return validators;
    }
};

struct ArgumentParser {

    Base_validator *get_validator(int nkey, const string &key) {
        Base_validator *ans;
        switch (nkey) {
            case 0://-inum
                ans = new Inum_validator(stoi(key));
                break;
            case 1://-name
                ans = new Name_validator(key);
                break;
            case 2://-size
                ans = new Size_validator(stoi(key.substr(1)), key[0]);
                break;
            case 3://-nlinks
                ans = new NLLink_validator(stoi(key));
                break;
            case 4://:-exec
            default:
                throw runtime_error("lol, it is impossible, but u made it...");
        }
        return ans;
    }

    Arguments parse(int argc, char **argv) {
        if (argc % 2) {
            end();
        }
        vector<string> keys(argc);
        for (int i = 0; i < argc; i++) {
            keys[i] = string(argv[i]);
        }
        Arguments args(keys[1]);
        for (int i = 2; i < argc; i += 2) {
            int nkey = -1;
            string const &key_arg = keys[i + 1];
            for (int j = 0; j < KEY_N; j++) {
                if ((j == 2 && (keys[i][0] == '+' || keys[i][0] == '-' || keys[i][0] == '=') &&
                     keys[i].substr(1) == KEYS[j]) ||
                    keys[i] == KEYS[j]) {
                    nkey = j;
                    break;
                }
            }
            if (nkey == -1) {
                end();
            } else if (nkey == 4) {
                args.add_exec(key_arg);
            } else {
                args.set_validator(get_validator(nkey, key_arg));
            }
        }
        return args;
    }
};

struct Find_engine {
private:

    vector<Base_validator *> validators;
    vector<string> found_files;
    vector<string> to_execute;
    string dir;

    bool is_dir(const string &pth) {
        struct stat st{};
        if (lstat(pth.c_str(), &st) == -1) {
            print_error("lstat failed");
            return false;
        }
        return (st.st_mode & S_IFMT) == S_IFDIR;
    }

    bool validate(const string &path, struct linux_dirent *dirent) {
        for (auto &validator : validators) {
            validator->set_fp(path);
            if (!validator->check(dirent))
                return false;
        }
        return true;
    }

    void bfs(const string &filepath) {
        int fd = open(filepath.c_str(), O_RDONLY | O_DIRECTORY);
        if (!check_error(fd, "open failed"))
            return;
        char buf[KB];
        vector<string> subdirs;
        int nread;
        struct linux_dirent *dirent;
        for (;;) {
            nread = syscall(SYS_getdents, fd, buf, KB);
            if (nread == -1) {
                print_error("getdents failed");
                break;
            }
            if (nread == 0)
                break;
            for (int bpos = 0; bpos < nread;) {
                dirent = (struct linux_dirent *) (buf + bpos);
                string s = string(dirent->d_name);
                if (s != "." && s != "..") {
                    string pth = filepath + string(dirent->d_name);
                    if (is_dir(pth)) {
                        subdirs.emplace_back(dirent->d_name);
                    } //else {
                    if (validate(pth, dirent))
                        found_files.emplace_back(pth);
                    //}
                }
                bpos += dirent->d_reclen;
            }
        }
        close(fd);
        for (const string &s : subdirs)
            bfs(filepath + s + "/");
    }

    void print_files() {
        for (string const &s: found_files)
            cout << s << endl;
    }

    void do_execute() {
        for (string const &s : to_execute) {
            for (string const &param : found_files)
                execute(s, param);
        }
    }

    void execute(const string &filepath, const string &param) {
        int return_code;
        __pid_t id = fork();
        if (id == -1) {
            print_error("fork failed");
        } else if (id != 0) {
            if (wait(&return_code) == -1) {
                print_error("wait failed");
            }
        } else {
            char **argus = new char *[1 + 2];
            argus[0] = new char[filepath.size() + 1];
            strcpy(argus[0], filepath.c_str());
            argus[0][filepath.size()] = '\0';
            argus[1] = new char[param.size() + 1];
            strcpy(argus[1], param.c_str());
            argus[1][param.size()] = '\0';
            argus[1 + 1] = nullptr;
            return_code = execve(argus[0], argus, nullptr);
            if (return_code == -1)//always true??
                print_error("execve failed");
            for (size_t i = 0; i < found_files.size() + 2; i++)
                delete[] argus[i];
            delete[] argus;
            exit(0);
        }

    }

public:
    explicit Find_engine(Arguments arguments) : validators(arguments.get_validators()),
                                                to_execute(arguments.get_exec()), dir(arguments.get_dir()) {}


    void walk() {
        bfs(dir);
        print_files();
        do_execute();
    }

    ~Find_engine() {
        for (auto &validator : validators)
            delete validator;
    }
};

int main(int argc, char **argv) {
    Find_engine(ArgumentParser().parse(argc, argv)).walk();
    return 0;
}
