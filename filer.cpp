#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <chrono>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <experimental/filesystem>
#define clrline write(1,"\033[2K",4)
//#define CTR(k) ((k) & 0x1f)



//-------------------------------cursor movements------------------------
#define c_save write(1,"\033[s",3)			//save cursor position
#define c_restore write(1,"\033[u",3)		//restore cursor position
#define up write(STDOUT_FILENO,"\033[1A",4)
#define down write(STDOUT_FILENO,"\033[1B",4)
#define left write(STDOUT_FILENO,"\033[1D",4)
#define right write(STDOUT_FILENO,"\033[1C",4)
#define c_pos(row,col) printf("\033[%d;%dH",row,col);\
						fflush(stdout)




//-----------------------------variable declaration------------------------
using namespace std;
using namespace chrono_literals;
int rows, columns;
int col = 1, row = 1;
struct termios reset, raw;
struct dirent *de;
string modes[] = {"-NORMAL MODE-" ,"-COMMAND MODE-"};
int mode = 0;
int lines = 0;
vector<vector<string>> vfile, vdir, vfull;
list<string> history;
string pwd;






//------------------------------TERMIOS MODES SETTINGS-------------------------------------
void resetall() {

	printf("\033[2J\033[;H\033[?47l\0338");
	fflush(stdout);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &reset);
}

void setraw() {
    tcgetattr(STDIN_FILENO, &reset);
    atexit(resetall);

    raw = reset;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | BRKINT);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}





void statusbar() {
    int i;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    rows = w.ws_row;
    columns = w.ws_col;

    c_pos((rows-1),1);
    clrline;
    c_pos(rows-1,columns-1);
    system("tput smso");
    write(1,"  ",1);
    c_pos((rows-1),1);
    write(STDOUT_FILENO,modes[mode].c_str(),modes[mode].size());
    for(i=modes[mode].size(); i<columns-20; i++){
        system("tput smso");
        write(1," ",1);
    }

    system("tput smso");
    printf("Files:%d, Folders:%d ", vfile.size(),vdir.size());
    c_pos(rows-1,columns);
    system("tput smso");
    write(1," ",1);
    fflush(stdout);
    system("tput rmso");

}



void fill_vecs() {
    DIR *dir = opendir(pwd.c_str());
    while ((de = readdir(dir)) != NULL) {
        vector<string> v(4);
        v[0] = "-" + string(de->d_name);
        string name = pwd + "/" + string(de->d_name);
        struct stat fileStat;
        stat(name.c_str(),&fileStat);
        string perms = string((fileStat.st_mode & S_IFDIR) ? "d" : "-") +
                        string((fileStat.st_mode & S_IRUSR) ? "r" : "-") +
                        string((fileStat.st_mode & S_IWUSR) ? "w" : "-") +
                        string((fileStat.st_mode & S_IXUSR) ? "x" : "-") +
                        string((fileStat.st_mode & S_IRGRP) ? "r" : "-") +
                        string((fileStat.st_mode & S_IWGRP) ? "w" : "-") +
                        string((fileStat.st_mode & S_IXGRP) ? "x" : "-") +
                        string((fileStat.st_mode & S_IROTH) ? "r" : "-") +
                        string((fileStat.st_mode & S_IWOTH) ? "w" : "-") +
                        string((fileStat.st_mode & S_IXOTH) ? "x" : "-");
        v[2] = perms;

        time_t cftime = fileStat.st_mtime;
        v[3] = asctime(localtime(&cftime));

        vector<string> sizes = {"", "k", "M", "G", "T"};
        int size = fileStat.st_size;
        int idx = 0;
        while (size > 1023) {
            size /= 1024;
            idx++;
        }
        v[1] = to_string(size) + sizes[idx] + "B";

        if (S_ISDIR(fileStat.st_mode)) 
            vdir.push_back(v);

        else 
            vfile.push_back(v);
    }

    sort(vdir.begin(), vdir.end());
    sort(vfile.begin(), vfile.end());

}




void prntscr(int sn) {
    sn += 1;
    write(1,"\033[J",3);
    cout<<pwd<<":"<<endl;

    fill_vecs();

    int i,j;
    for (i=0;i<vdir.size();i++)
        for(j=0;j<4;j++) 
            cout<<vdir[i][j]<<"    ";
    for (i=0;i<vfile.size();i++)
        for(j=0;j<4;j++) 
            cout<<vfile[i][j]<<"    ";
    
    statusbar();
    write(STDOUT_FILENO,"\033[H",3);
}





int main(int arg, char const *args[]) {
    printf("\E7\E[?47h");
    fflush(stdout);
    setraw();

    signal(SIGWINCH,prntscr);
    write(STDOUT_FILENO,"\033[H",3);

    char path[200];
    getcwd(path, 200);
    pwd = string(path);

    if (arg==2) {
        if (args[1][0] == '/' || args[1][0] == '~' )
            pwd = string(args[1]);
        else
            pwd += "/" + string(args[1]);
    }

    prntscr(0);

    // struct stat fileStat;
    // if(stat(pwd.c_str(),&fileStat) < 0)    
    //     return 1;
 
    // printf("Information for %s\n",pwd.c_str());
    // printf("---------------------------\n");
    // printf("File Size: \t\t%d bytes\n",fileStat.st_size);
    // printf("Number of Links: \t%d\n",fileStat.st_nlink);
    // printf("File inode: \t\t%d\n",fileStat.st_ino);
 
    // printf("File Permissions: \t");
    // printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    // printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    // printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    // printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    // printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    // printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    // printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    // printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    // printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    // printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
    // printf("\n\n");



    // DIR *dir = opendir(pwd.c_str());
    // while ((de = readdir(dir)) != NULL) {
    //     string file = de->d_name;
    //     if(file != "." && file != ".."){
    //         struct stat st;
    //         stat((pwd+file).c_str(), &st);
    //         S_ISDIR(st.st_mode) ? printf("%s\n", file.c_str()) : printf("-\n");
    //     }
    // }
    

    while(1) {
        char c;
        read(STDIN_FILENO, &c, 1);

        switch(c) {

            // case ':':           //command mode
            //     mode=1;
            //     c_save;
            //     c_mode();
            //     mode=0;
            //     prntscr(1);
            //     c_restore;
            //     c_save;
            //     c_pos(rows,1);
            //     c_restore;

            //     break;

            // case 27:            //arrow keys
            //     char s[2];
            //     read(STDIN_FILENO, &s, 2);
            //     switch(s[1]) {
            //         case 'A':           //up
            //             move_up();
            //             break;
            //         case 'B':           //down
            //             move_down();
            //             break;
            //         case 'C':           //right
            //             move_right();
            //             break;
            //         case 'D':           //left
            //             move_left();
            //             break;
            //     }
            //     break;

            case 'q':
                exit(0);

        }
    }
    return 0;
}