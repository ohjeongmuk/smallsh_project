#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

#define LENGTH 2048

volatile sig_atomic_t tstp = false;
volatile sig_atomic_t mode = false;
volatile sig_atomic_t background = false;
volatile sig_atomic_t pass = false;
volatile sig_atomic_t isRedirect = false;
int exit_status = 0;

char* replaceAll(char *s, const char *olds, const char *news);
void cd_command(char** args, int end_idx);
int others(char** args, int end_idx);
void run_background(char** args, int end_idx);
void status_command(int result);
void redirection(char* args[512], int end_idx);

void handle_SIGINT (int signo) {
    char* msg = "terminated by signal 2\n";
    write(STDOUT_FILENO, msg, 23);
    if (!background) exit_status = 2;
}

// pause()함수는 재진입 불가능한 함수이기 때문에 사용 불가능하다.
void handle_SIGTSTP (int signo) {
    char* msg1 = "Entering foreground-only mode (& is now ignored)\n";
    char* msg2 = "Exiting foreground-only mode\n";
    char* prompt = ": ";
    // 포그라운드로 실행중인 자식 프로세스가 존재할때 부모 프로세스는 pass 를 true로 설정한다.
    // 그리고 다음 커맨드라인 처리를 위해서 tstp를 다시 false로 설정한다.
    if (tstp) {
        pass = true;
        tstp = false;
    // 자식 프로세스에서 아무런 커맨드 라인을 실행하지 않는다면 write()함수로 출력
    } else {
        if (mode) {
            write(STDOUT_FILENO, msg2, 29);
            write(STDOUT_FILENO, prompt, 2);
            mode = false;
        } else {
            write(STDOUT_FILENO, msg1, 49);
            write(STDOUT_FILENO, prompt, 2);
            mode = true;
        }
    }
}

int main(void) {
    // getpid는 현재 프로세스 아이디를 구한다.
    int cur_pid = getpid();
    char pid_s[10];
    sprintf(pid_s, "%d", cur_pid);
    int result = 0;
    int temp;
    size_t len;
    
    // TODO: 유저에게 커맨드라인 입력받고 토큰으로 쪼개기
    char command_line[LENGTH];
    // 512개의 문자열 포인터가 존재한다
    char* args[512];
    int i = 0;
    int j = 0;
    int* end_idx = &i;

    struct sigaction SIGTSTP_action = {0}, ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;

    // TODO: SIGTSTP 시그널 핸들링 처리하기
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;   // SA_RESTART 안해주면 이상하게 에러가 뜸, 그리고 이것은 scanf 등등.. 관련이 있는듯
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    // TODO: SIGINT 시그널 핸들링 처리하기
    while(1) {
        sigaction(SIGINT, &ignore_action, NULL);

        printf(": ");
        fflush(stdout);
        // 띄어씌기를 포함해서 입력받는다.
        if (fgets(command_line, sizeof(command_line), stdin) == NULL) {
            // 입력 오류 또는 EOF에 도달한 경우 루프를 종료합니다.
            break;
        }

        // command_line에서 개행 문자 제거
        command_line[strcspn(command_line, "\n")] = '\0';

        // 주석 처리 #
        if (command_line[0] == '#') {
            fflush(stdout);
            fflush(stdin);
            continue;
        }
        // $$ 명령어 확장: commmand_line 안에는 $$가 Pid로 대체된 라인으로 교환됨
        strcpy(command_line, replaceAll(command_line, "$$", pid_s));

        // command_line 자르기 -> args문자열 배열에 모두 저장
        char* token = strtok(command_line, " "); 
        while(token != NULL) {
            if (strcmp(token, ">") == 0 || strcmp(token, "<") == 0) isRedirect = true;
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if(mode & (strcmp("&", args[i-1]) == 0)) {
            args[i-1] = malloc(sizeof((0)));
            args[i-1] = NULL;
            *end_idx = *end_idx - 1;
        }
        // 이것을 여기에 하는 이유는, 바로 버퍼를 비워줘야하기 때문에
        fflush(stdout);
        fflush(stdin);

        // 첫번째 문자열로 command를 구분
        if(strcmp(args[0], "cd") == 0) {
            cd_command(args, (*end_idx)-1); 
        }else if (strcmp(args[0], "status") == 0) {
            status_command(result);
        }else if (strcmp(args[0], "exit") == 0) {
            exit(0);
        } else {
            temp = result;
            result = others(args, (*end_idx)-1);
            if (background) result = temp;
        }
    i = 0;
    background = false;
    isRedirect = false;
    fflush(stdout);
    fflush(stdin);
    }
    return 0;
}

int others(char** args, int end_idx) {
    pid_t spwanPID;
    int childExitMethod;
    int backgroundStatus = 0;
    int t;

    // others에서 SIGTSTP 처리하기
    struct sigaction SIGTSTP_action = {0}, SIGINT_action = {0}, default_action = {0};
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;       // TSTP는 restart 해준다.
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    // defaultSIG 설정
    default_action.sa_handler = SIG_DFL;

    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;
    SIGINT_action.sa_handler = handle_SIGINT;
    sigaction(SIGINT, &SIGINT_action, NULL);
    // 백그라운드인지 포그라운드인지 확인한다
    if (strcmp(args[end_idx], "&") == 0) background = true;
    else background = false;

    spwanPID = fork();
    switch(spwanPID) {
    case -1:
        perror("spawanPID failed\n");
        exit(1);
        break;
    // 여기는 자식 프로세스에서 실행된다
    case 0:
        // 하위 프로세스에서 실행되는 포그라운드/백그라운드는 SIGTSTP를 무시한다.
        SIGTSTP_action.sa_handler = SIG_IGN;
        SIGINT_action.sa_flags = 0;
        sigaction(SIGTSTP, &SIGTSTP_action, NULL);

        // 리다이렉션 먼저
        if (isRedirect) {
            redirection(args, end_idx);
            break;
        } else if (background) {    // 
            // 백그라운드에서 SIGINT 무시한다
            // 미리 &를 없애준다
            args[end_idx] = (char*)0;
            // 자식: 백그라운드 SIGINT처리 -> 무시한다
            SIGINT_action.sa_handler = SIG_IGN;
            sigaction(SIGINT, &SIGINT_action, NULL);

            run_background(args, end_idx-1);
            exit(0);
            break;
        } else { //포그라운드 실행
            // 자식 프로세스에서는 SIGINT시그널 핸들링: 메세지를 출력하고 종료
            sigaction(SIGINT, &default_action, NULL);
            // execvp() 프로그램 대체
            execvp(args[0], args);
            printf("bash: %s: No such file or directory\n", args[0]);
            fflush(stdout);
            exit(1);
            break;    
        } 
    default:
        // 부모 프로세스에서 처리한다.
        tstp = true;

        if(background) { // 백그라운드로 실행될 때
            // SIGINT SIGTSTP 모두 무시
            SIGTSTP_action.sa_handler = SIG_IGN;
            sigaction(SIGTSTP, &SIGTSTP_action, NULL);
            SIGINT_action.sa_handler = SIG_IGN;
            sigaction(SIGINT, &SIGINT_action, NULL);

            waitpid(spwanPID, &childExitMethod, WNOHANG);
            fflush(stdout);
            fflush(stdin);
        } else { // 포그라운드에서 실행될 때 & 리다이렉션이 실행 될 때
            waitpid(spwanPID, &childExitMethod, 0);
            if(pass) {
                if (mode) {
                    printf("Exiting foreground-only mode\n");
                    mode = false;
                }
                else {
                    printf("Entering foreground-only mode (& is now ignored)\n");
                    mode = true;
                }
                pass = false;
            }    
            fflush(stdout);
            fflush(stdin);         
            
        }
    }

    return childExitMethod;
} 

void run_background(char** args, int end_idx) {
    int childExitMethod;
    // 2차 자식에서 SIGTSTP 처리하기
    struct sigaction SIGTSTP_action = {0}, SIGINT_action = {0};
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_handler = SIG_IGN;
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    // 2차 자식에서 SIGINT 처리하기
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_handler = SIG_IGN;
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    pid_t backgroundPID = fork();
    switch(backgroundPID) {
    case -1:
        perror("backgroundPID failed\n");
        fflush(stdout);
        exit(1);
        break;
    case 0:
        printf("background pid is %d\n", getpid());
        fflush(stdout);
        fflush(stdin);
        execvp(args[0], args);
        exit(1);
        break;
    default:
        waitpid(backgroundPID, &childExitMethod, 0);
        // 종료가 시그널에 의해서 비정상으로 종료 될 경우에만 실행
        printf("\nbackground pid %d is done.\n", backgroundPID);
        if (WIFEXITED(childExitMethod)) printf("exit value %d\n", WEXITSTATUS(childExitMethod));
        else if (WIFSIGNALED(childExitMethod)) printf("terminated by singal %d\n", WTERMSIG(childExitMethod));
        fflush(stdout);
    }
}

// args는 읽기용이다.
void cd_command(char** args, int end_idx) {
    char dir[1024];
    // command: cd
    if(args[1] == NULL) {
        getcwd(dir, sizeof(dir));
        chdir(getenv("HOME"));
    } else  
        // command: cd dir_name
        chdir(args[1]);
}

// $$
char *replaceAll(char *s, const char *olds, const char *news) {
  char *result, *sr;
  size_t i, count = 0;
  size_t oldlen = strlen(olds); if (oldlen < 1) return s;
  size_t newlen = strlen(news);

  if (newlen != oldlen) {
    for (i = 0; s[i] != '\0';) {
      if (memcmp(&s[i], olds, oldlen) == 0) count++, i += oldlen;
      else i++;
    }
  } else i = strlen(s);

  result = (char *) malloc(i + 1 + count * (newlen - oldlen));
  if (result == NULL) return NULL;


  sr = result;
  while (*s) {
    if (memcmp(s, olds, oldlen) == 0) {
      memcpy(sr, news, newlen);
      sr += newlen;
      s  += oldlen;
    } else *sr++ = *s++;
  }
  *sr = '\0';

  return result;
}

void status_command(int result) {
    int termSignal;
    if (WIFEXITED(result)) {
        // 정상적으로 종료되었을 경우: 0
        printf("exit value %d\n", WEXITSTATUS(result));
    } else if (WIFSIGNALED(result)) {
        // 시그널에 의해 종료된 경우
        termSignal = WTERMSIG(result);
        printf("terminated by signal %d\n", termSignal);
    }
    fflush(stdout);
    fflush(stdin);
}

void redirection(char* args[512], int end_idx) {
    int fp;
    int childExitMethod;

    for (int i = 0; i < end_idx; ++i) {
        if (!strcmp(args[i], ">")) {
            args[i] = NULL;
            // attempt to open output file
            if ((fp = open(args[++i], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
                printf("bash: %s No such file or directory\n", args[i]);
                fflush(stdout); 
            }
            // change output to output file
            dup2(fp, 1);
            close(fp);
        } else if (!strcmp(args[i], "<")) {
            args[i] = NULL;
            // attempt to open input file
            if ((fp = open(args[++i], O_RDONLY)) < 0) {
                printf("bash: %s No such fiile or directory\n", args[i]); 
                fflush(stdout);
            }
            // change input to input file
            dup2(fp, 0);
            close(fp);
        }
    }
    execvp(args[0], args);
    perror(args[0]);
}