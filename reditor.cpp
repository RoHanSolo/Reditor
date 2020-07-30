/* Roll no.: 20172038
 * Rohan Gandhi
 */


#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <bits/stdc++.h>
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
int rows, columns;
int col=1,row=1;
struct termios reset,raw;
string modes[] = {"-NORMAL MODE-","-INSERT MODE-","-COMMAND MODE-"};
int mode=0;
int saved=0;
int c_case;
int lines=0;
list <string> data, gbuf;
list <string> :: iterator g_it,  bstart_it, bend_it;
int start_p=0, end_p=0;
int total_lines=0;
int letternum=0;
fstream file;
string filename="";




//------------------------------TERMIOS MODES SETTINGS-------------------------------------
void resetall() {

	printf("\033[2J\033[;H\033[?47l\0338");
	fflush(stdout);

	if(file)
		file.close();

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




//------------------------------------SCREEN RENDERING------------------------------
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
	for(i=modes[mode].size();i<(columns-20);i++){
		system("tput smso");
		write(1," ",1);
	}

	system("tput smso");
	printf("ROWS:%d COLUMNS:%d", rows,columns);

	fflush(stdout);
	system("tput rmso");
}

void prntscr(int status) {
	list<string>::iterator i;
	c_pos(1,1);
	// write(1,"\033[J",3);
	for(i=gbuf.begin();i!=gbuf.end();i++) {
		clrline;
		string str = *i;
		write(1,str.c_str(),str.size());
	}

	if(status)
		statusbar();
	c_pos(1,1);
}

void fill_gbuf(int status) {   //Filling the graphics buffer

	int lines=0;
	int len;
	if(mode!=1)
		col=1;
	bend_it = bstart_it;
	gbuf.clear();
	while(lines<rows-2 && bend_it!=data.end()){
		len = (*bend_it).length()-start_p;
		if(len>columns) {
			while (len>0 && lines<rows-2) {
				gbuf.push_back((*bend_it).substr(((*bend_it).length()-len),columns));
				len-=columns;
				if(len<0)	len=0;

				end_p = (*bend_it).length()-len;
				lines++;
			}
			if(len==0)	bend_it++;
		}
		else {
			gbuf.push_back((*bend_it));
			bend_it++;
			lines++;
		}
	}

	g_it = gbuf.begin();
	prntscr(status);
}

void resize(int sn) {
	sn+=1;
	c_save;
	c_pos(rows-1,1);
	clrline;
	fill_gbuf(1);
	c_restore;
}





//--------------------------------CURSOR MOVEMENT--------------------------------------
void scrolldown() {
	string str = *bstart_it;
	if((int)(str.substr(start_p,str.length())).length()>columns) {
		start_p+=columns;
	}

	str = *bend_it;
	if(end_p == (int)str.length()) {
		return;
	}
	else {
		str = str.substr(end_p,columns);

		int len = (int)(*bend_it).length();

		if((end_p+columns)<len)
			end_p = end_p + columns;
		else
			end_p=len;

	}
	gbuf.pop_front();
	gbuf.push_back(str);
	prntscr(0);

}

void scrollup() {

	if(start_p==0) {
		c_save;
		c_pos(rows,1);
		clrline;
		write(1,"Beginning of line!",18);
		c_restore;
		return;
	}

	letternum--;
	col = columns;
	string str = *bstart_it;
	start_p-=columns;
	str = str.substr(start_p,columns);

	if(end_p == (int)(*bend_it).length()){
		end_p -= (*bend_it).length()%columns;
		if(end_p==0) {
			bend_it--;
		}
	}
	else {
		end_p -= columns;
	}
	gbuf.push_front(str);
	gbuf.pop_back();
	prntscr(0);
}

void move_right() {

	if(data.size()!=0) {
		int end = (*g_it).find('\n');

		if(col==end+1) {
			c_save;
			c_pos(rows,1);
			clrline;
			write(1,"End of line!",12);
			c_restore;
		}
		else {
			letternum++;
			if(col == columns) {
				col=1;
				if(row == rows-2) {
					scrolldown();
					g_it++;
				}
				else {
					row++;
					g_it++;
				}
			}
			else
				col++;
			c_pos(row,col);
		}
	}
}

void move_left() {

	if(data.size()!=0) {
		if(col == 1) {
			if(row == 1) {
				scrollup();
			}
			else{
				col = columns;
				row--;
				letternum--;
				g_it--;
			}
		}
		else {
			col--;
			letternum--;
		}

		c_pos(row,col);
	}
}

void move_down() {
	if(data.size()!=0) {
		if(bstart_it!=data.end())
			bstart_it++;
		if(bstart_it!=data.end())
			fill_gbuf(0);
		row = col = 1;
		g_it = gbuf.begin();
		letternum=0;
	}
}

void move_up() {
	if(data.size()!=0) {
		if(bstart_it!=data.begin())
			bstart_it--;
		fill_gbuf(0);
		row = col = 1;
		g_it = gbuf.begin();
		letternum=0;
	}
}






/*-----------------------INSERT MODE-----------------------*/
void i_mode() {
	saved=0;
	c_save;
	statusbar();
	c_restore;
	char c;
	while(1) {
		read(0,&c,1);

		if(data.size()!=0) {
			string pre, post;
			switch(c) {
				case 127:
//					//backspace
//					if(end_p==(int)(*bstart_it).length())
//						end_p++;
//					c_save;
//					c_pos(rows,1);
//					printf("%d ",letternum);
//					fflush(stdout);
//					c_restore;
					if(letternum) {
						pre = (*bstart_it).substr(0,letternum-1);
						post = (*bstart_it).substr(letternum,(int)(*bstart_it).length());
						(*bstart_it) = pre+post;
						letternum--;
						if(col==1) {
							if(row==1) {

								if(start_p==0) {

								}
								else {
									col=columns;
									start_p-=columns;
								}
							}
							else {
								col=columns;
								row--;
							}
						}
						else{
							col--;
						}
					}
					c_pos(row,col);
					write(1," ",1);
					break;

				case '\n':
					//new node
					break;

				case 27:
					return;

				default:
					pre = (*bstart_it).substr(0,letternum);
					post = (*bstart_it).substr(letternum,(int)(*bstart_it).length());
					(*bstart_it) = pre+c+post;
					letternum++;
					if(end_p!=((rows-2)*columns))
						end_p++;

					if(col==columns){
						col=1;
						if(row!=rows-2)
							row++;
						else
							start_p+=columns;
					}
					else
						col++;
					break;
			}
		}
		else {
			if(c==27)
				return;
			else {
				string str = " \n";
				str[0] = c;
				data.push_back(str);
				letternum++;
				col++;
				end_p = (int)str.length();
				c_pos(row,col);
				bstart_it=data.begin();
			}
		}
		fill_gbuf(0);
		g_it=gbuf.begin();
		for(int i=1;i<row;i++) {
			g_it++;
		}
		c_pos(row,col);
	}
}





/*---------------------------------COMMAND MODE--------------------------------------*/
void c_mode() {
	char c;
	statusbar();
	printf("\033[%d;1H\033[K\r:",rows);
	fflush(stdout);
	while(read(0,&c,1) && c!=27) {

		//--------------------------------system commands-----------------------*/
		if(c=='!') {
			write(1,"!",1);
			string command = "";
			while(read(0,&c,1) && c!='\n') {
				if(c==127) {
					printf("\b \b");
					fflush(stdout);
					command = command.substr(0,(command.size()-1));
					continue;
				}
				else if(c==27){
					if(read(0, &c, 1) && c=='[')
						continue;
					write(1,"\033[B",1);
					for(int i=0;i<columns;i++)
						write(1," ",1);
					return;
				}
				write(1,&c,1);
				command += c;
			}

			pid_t proc;
			proc = fork();
			if (proc==0) {
				printf("\E[?47l\E8");
				fflush(stdout);
				system("tput bold");
				printf("\r\n\n%s\n\n\r",command.c_str());
				fflush(stdout);
				system("tput sgr0");
				execlp("/bin/bash","bash","-c",command.c_str(),(char *)0);
			}
			else {
				wait(&proc);
				printf("\r\nPress ENTER to continue...\n\r\n");
				fflush(stdout);

				while(read(0,&c,1) && c!='\n') {}
				printf("\E7\E[?47h");
				fflush(stdout);
			}

//			system(command.c_str());

			return;
			/*for(int i=0;i<=columns;i++)
				write(1," ",1);*/

		}

		/*------------------------------self commands--------------------------*/
		else if(c=='q') {
			write(1,&c,1);
			read(0,&c,1);
			int force=0;
			while(1) {
				if(c=='!') {
					write(1,&c,1);
					force=1;		//set force quit
					read(0,&c,1);
				}

				if(c=='\n') {
					if(force==1) {
						exit(0);
					}
					if(force==0){
						//check if file is saved
						switch(saved) {
							case 0:
								c_pos(rows,1);
								clrline;
								c_case=1;
								return;
							case 1:
								exit(0);
						}
					}
				}

				if(c==27) {
					clrline;
					return;
				}
			}
		}

		else if(c=='w') {
			write(1,&c,1);
			while(read(0,&c,1) && c!='\n') {
				if(c==27) {
					clrline;
					return;
				}
			}

			if(filename!="") {		//for no arguments
				file.close();
				file.open(filename.c_str(),ios::out|ios::trunc);
			}
			else {
				write(1,"Enter filename: ",15);
				while(read(0,&c,1) && c!='\n') {
					write(1,&c,1);
					filename+=c;
				}
				file.open(filename.c_str());
				file.close();
				file.open(filename.c_str(),ios::out|ios::trunc);
			}

			list<string>::iterator it;
			for(it = data.begin();it!=data.end();it++)
				file << (*it);
			file.close();

			c_case=2;
			saved = 1;
			clrline;
			file.open(filename.c_str(), ios::out|ios::in);
			return;
		}
	}
	clrline;
}






/*---------------------NORMAL MODE + DRIVER PROGRAM-----------------*/

int main(int arg, char *args[]) {
	printf("\E7\E[?47h");
	fflush(stdout);
    setraw();

	statusbar();
	signal(SIGWINCH,resize);
	write(STDOUT_FILENO,"\033[H",3);

    ofstream newfile;

    if(arg==2) {
        filename = args[1];
        file.open(filename.c_str(),ios::in|ios::out);
        if(!file.good()) {
        	newfile.open(filename.c_str());
        	newfile.close();
            file.open(filename.c_str(),ios::in|ios::out);
        }

		string line;

		while(getline(file,line)) {
			line+="\n";
			total_lines++;
			data.push_back(line);
		}

		bstart_it = data.begin();

		fill_gbuf(0);

    }

	write(STDOUT_FILENO, "\033[H", 3);

    while(1) {
        char c;
        read(STDIN_FILENO, &c, 1);

        switch(c) {

            case 'i':			//insert mode
            	mode=1;
            	i_mode();
            	mode=0;
            	c_save;
            	statusbar();
            	c_restore;
                break;

            case 'h':			//left
                move_left();
                break;

            case 'j':			//down
            	move_down();
                break;

            case 'k':			//up
            	move_up();
                break;

            case 'l':			//right
            	move_right();
            	break;

            case ':':			//command mode
            	mode=2;
            	c_save;
                c_mode();
            	mode=0;
            	prntscr(1);
            	c_restore;
            	c_save;
            	c_pos(rows,1);
            	if(c_case==1)
            		write(1,"File not saved!",15);
            	else if(c_case==2)
            		write(1,"File saved!",11);
            	c_restore;

                break;

            case 'r':			//replace character
            	saved = 0;

            	c_save;
            	c_pos(rows,1);
            	write(1,"Enter replace character:",24);
            	c_restore;
            	read(0,&c,1);
            	write(1,&c,1);

            	(*bstart_it)[letternum] = c;
            	(*g_it)[col-1] = c;
            	left;
            	c_save;
            	c_pos(rows,1);
            	clrline;
            	c_restore;
            	break;

            case 27:			//arrow keys
				char s[2];
                read(STDIN_FILENO, &s, 2);
                switch(s[1]) {
                    case 'A':			//up
        				move_up();
						break;
                    case 'B':			//down
                        move_down();
						break;
                    case 'C':			//right
                    	move_right();
						break;
                    case 'D':			//left
                        move_left();
						break;
                }
				break;

            case 'g':					//beginning of file
                if(read(STDIN_FILENO, &c, 1) && c=='g'){
                	bstart_it=data.begin();
                	fill_gbuf(0);
                }
                else
                	continue;
                break;

            case 'G':					//end of file
                bstart_it=data.end();
                bstart_it--;
                fill_gbuf(0);
                break;

//			case CTR('q'):
//				goto end;

        }
    }

//	end:
    return 0;
}
