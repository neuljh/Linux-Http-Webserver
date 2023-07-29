#include <pthread.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <sys/wait.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>
#include<netdb.h>
#include<stdlib.h>
#include<string>
#include<iostream>
#include<time.h>
#include<string.h>
#include <fstream>

using namespace std;

#define BUF_SIZE 1024
#define SMALL_BUF 100
#define QUEUE_SIZE 128

void* request_handler(void* arg);
void send_data(FILE* fp, char* ct, char* file_name);
char* content_type(char* file);
void send_error(FILE* fp);
void error_handling(char* message);
int check_path(char* file_path);
void child_waiter(int signum);

bool not_exist(char* arg);
void do_404(char* arg,FILE *fp);
bool is_dir(char* arg);
void do_ls(char* arg,FILE *fp);
bool ends_in_cgi(char* arg);
void do_exec(char* arg,FILE *fp);
void do_cat(char* arg,FILE *fp);
void do_pic(char* arg,FILE *fp);
bool ends_in_html(char* arg);
bool ends_in_pic(char* arg);
int sendPic(FILE *f,char* arg);

int get_char_times(char* str,char c);
void strmcpy(char *p, char *a, int m);
void get_time(char* time_str);
void log_fun(FILE *log_fp,char *time_log,char *temp);

int make_server_socket(int portnum);
int connect_to_server(char* host,int portnum);

int main(int argc, char *argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_size;
	char buf[BUF_SIZE];
	pthread_t t_id;	

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	signal(SIGCHLD,child_waiter);
	
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 20)==-1)
		error_handling("listen() error");
	// if((serv_sock=make_server_socket(atoi(argv[1])))==-1){
    //     //printf("hello1");
    //     exit(1);
    // }
	FILE *log_fp=fopen("log.txt","a+");
    if(log_fp==NULL){
        fprintf(stderr,"ERROR:Create a log file unsuccessfully!");
        exit(1);
    }
    fclose(log_fp);

	while(1)//gcc webserv_linux.c -D_REENTRANT -o web -lpthread;g++ webserv_linux.cpp -D_REENTRANT -o web -lpthread
	{
		clnt_adr_size=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, (socklen_t*)&clnt_adr_size);
		printf("Waiting for the client to connect......\n");
		printf("Connection Request : %s:%d\n", 
			inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));

		// FILE* fpin=fdopen(dup(clnt_sock),"r");
		// char request[128];
        // fgets(request,128,fpin);
		
		// printf("0\n");
        // printf("Request = %s\n",request);
		// fclose(fpin);

		pthread_create(&t_id, NULL, request_handler, &clnt_sock);
		
		pthread_detach(t_id);
	}
	close(serv_sock);
	return 0;
}

void* request_handler(void *arg)
{
	int clnt_sock=*((int*)arg);
	char req_line[SMALL_BUF];
	char *request;
	FILE* clnt_read;
	FILE* clnt_write;
	
	char method[10];
	char ct[15];
	char file_name[30];
  
	clnt_read=fdopen(clnt_sock, "r");
	clnt_write=fdopen(dup(clnt_sock), "w");

	fgets(req_line, SMALL_BUF, clnt_read);
	printf("req_line111=%s\n",req_line);
	request=req_line;	
	
	printf("req_line112=%s\n",req_line);
	printf("request=%s\n",request);

	char time[SMALL_BUF];
	FILE *log_fp=fopen("log.txt","a+");

	log_fun(log_fp,time,request);
	printf("time= %s\n",time);
	printf("req_line1=%s\n",req_line);
	printf("req_line2=%s\n",strstr(req_line, "HTTP/"));

	if(strstr(req_line, "HTTP/")==NULL){
		printf("error: HTTP/1.0 400 Bad Request\n");
		send_error(clnt_write);
		fclose(clnt_read);
		fclose(clnt_write);
		exit(0);
		// return;
	}
	printf("req_line=%s\n",req_line);//1
	strcpy(method, strtok(req_line, " /"));
	printf("method=%s\n",method);//2
	strcpy(file_name, strtok(NULL, " "));
	
	// strcpy(ct, content_type(file_name));

	if(get_char_times(file_name,'/')==1){
		char fn[strlen(file_name)];
		strmcpy(file_name,fn,1);
		strcpy(file_name,fn);
	}

	printf("file_name=%s\n",file_name);
	printf("file_return:%d\n",check_path(file_name));

	int sign=0;

	if(strcmp(method, "GET")!=0){

		sign=1;

		send_error(clnt_write);
		fclose(clnt_read);
		fclose(clnt_write);
		//return;
		//exit(0);
	}else if(not_exist(file_name)){

		sign=2;

		do_404(file_name,clnt_write);
		fclose(clnt_read);
		fclose(clnt_write);
		//return;
		//exit(0);
	}else if(is_dir(file_name)){

		sign=3;

		do_ls(file_name,clnt_write);
		//printf("1\n");
		fclose(clnt_read);
		//printf("2\n");
		fclose(clnt_write);
		//printf("3\n");
		//return;
		//exit(0);
	}else if(ends_in_cgi(file_name)){

		sign=4;

		do_exec(file_name,clnt_write);
		fclose(clnt_read);
		fclose(clnt_write);
		//return;
		//exit(0);
	}else if(ends_in_html(file_name)){

		sign=5;
		strcpy(ct, content_type(file_name));
		send_data(clnt_write, ct, file_name);
		fclose(clnt_read);
		fclose(clnt_write);
		//return;
		//exit(0);
	}else if(ends_in_pic(file_name)){
		
		sign=6;
		//do_pic(file_name,clnt_write);
		sendPic(clnt_write,file_name);
		fclose(clnt_read);
		fclose(clnt_write);
	}
	
	else{

		sign=7;

		do_cat(file_name,clnt_write);
		fclose(clnt_read);
		fclose(clnt_write);
		//return;
		//exit(0);
	}
	printf("req_line=%s;method=%s;file_name=%s;ct=%s\n",req_line,method,file_name,ct);
	printf("\n sign=%d\n",sign);
	//fclose(clnt_read);
	//fclose(clnt_write);

	
}

void send_data(FILE* fp, char* ct, char* file_name)
{
	printf("send data start: HTTP/1.0 200 OK\n");
	char protocol[]="HTTP/1.0 200 OK\r\n";
	char server[]="Server:Linux Web Server \r\n";
	char cnt_len[]="Content-length:2048\r\n";
	char cnt_type[SMALL_BUF];
	char buf[BUF_SIZE];
	FILE* send_file;
	
	sprintf(cnt_type, "Content-type:%s\r\n\r\n", ct);
	send_file=fopen(file_name, "r");
	if(send_file==NULL)
	{
		send_error(fp);
		return;
	}

	fputs(protocol, fp);
	fputs(server, fp);
	fputs(cnt_len, fp);
	fputs(cnt_type, fp);

	while(fgets(buf, BUF_SIZE, send_file)!=NULL) 
	{
		fputs(buf, fp);
		fflush(fp);
	}
	fflush(fp);
	printf("send data end\n");
}

char* content_type(char* file)
{
	char extension[SMALL_BUF];
	char file_name[SMALL_BUF];
	strcpy(file_name, file);
	strtok(file_name, ".");
	strcpy(extension, strtok(NULL, "."));
	
	if(!strcmp(extension, "html")||!strcmp(extension, "htm")) 
		return "text/html";
	else
		return "text/plain";
}

void send_error(FILE* fp)
{	
	char protocol[]="HTTP/1.0 400 Bad Request\r\n";
	char server[]="Server:Linux Web Server \r\n";
	char cnt_len[]="Content-length:2048\r\n";
	char cnt_type[]="Content-type:text/html\r\n\r\n";
	char content[]="404 Not Found\r\n"
		   "Unknown protocol";
		
	fputs(protocol, fp);
	fputs(server, fp);
	fputs(cnt_len, fp);
	fputs(cnt_type, fp);
	fflush(fp);
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int check_path(char* file_path){
    struct stat infos;
    if(stat(file_path,&infos)!=0){
        return -1;
    }else if(infos.st_mode & S_IFDIR){
        return 0;
    }else if(infos.st_mode & S_IFREG){
        return 1;
    }else{
        return -1;
    }
}

void child_waiter(int signum)
{
	wait(NULL);
}

bool not_exist(char* arg){
    // if(!strcmp(arg,"\0")){
    //     return true;
    // }
	if(check_path(arg)==-1){
        return true;
    }
    return false;
}

void do_404(char* arg,FILE *fp){
    fprintf(fp,"HTTP/1.0 404 Not Found\r\n");
    fprintf(fp,"Content-type:text/plain\r\n");
    fprintf(fp,"\r\n");
    fprintf(fp,"404 Not Found\r\n");
    fprintf(fp,"The origin server could not find a current representation for the target resource or is not willing to disclose that one exists.(%s)\r\n",arg);

	// int fd=fileno(fp);//test
	// dup2(fd,STDOUT_FILENO);
    // //dup2(fd,2);
    // // close(fd);
    // execl("/bin/ls","ls","-l",NULL);
}

bool is_dir(char* arg){
    if(check_path(arg)==0){
        return true;
    }
    return false;
}

void do_ls(char* arg,FILE *fp){
	
	int fd=fileno(fp);
    // header(fp,"text/plain");
	fprintf(fp,"HTTP/1.0 200 OK\r\n");
    fprintf(fp,"Content-type:text/plain");
    fprintf(fp,"\r\n\r\n");
    fflush(fp);

	printf("store origin STDOUT_FILENO\n");
	int savefd = dup(STDOUT_FILENO);
	printf("redirect towards server\n");
    dup2(fd,STDOUT_FILENO);
   
    //printf("do some work\n");
	pid_t sub=fork();
	if(sub==0){
		execl("/bin/ls","ls","-l",arg,NULL);
	}
    
	dup2(savefd, STDOUT_FILENO);
    printf("redirect towards STDOUT_FILENO\n");
}

bool ends_in_cgi(char* arg){
    const char *pFile=strrchr(arg,'.');
    if(pFile!=NULL){
        if(strcmp(pFile,".cgi")==0){
            return true;
        }
    }
    return false;
}

bool ends_in_html(char* arg){
    const char *pFile=strrchr(arg,'.');
    if(pFile!=NULL){
        if(strcmp(pFile,".html")==0||strcmp(pFile,".ico")==0){
            return true;
        }
    }
    return false;
}

bool ends_in_pic(char* arg){
    const char *pFile=strrchr(arg,'.');
    if(pFile!=NULL){
        if(strcmp(pFile,".jpg")==0||strcmp(pFile,".png")==0){
            return true;
        }
    }
    return false;
}

void do_exec(char* arg,FILE *fp){
	printf("send data start: HTTP/1.0 200 OK\n");
	FILE* send_file=fopen(arg,"rb");
	char* readBuf=(char*)malloc(1024*1024*10);
	int tlen,len;

	if(send_file==NULL){
		printf("fail to open file: %s\n",arg);
		return;
	}
	fseek(send_file,0,SEEK_END);
	len=ftell(send_file);
	if(len<=0){
		printf("fail to read file: %s\n",arg);
		return;
	}
	fseek(send_file,0,SEEK_SET);

	len=fread(readBuf,1,len,send_file);
	fclose(send_file);

	char* p_bufs=(char*)malloc(len+1024);
	if(p_bufs==NULL){
		printf("fail to malloc for file: %s\n",arg);
		return;
	}

	tlen = sprintf(p_bufs,	"HTTP/1.1 200 OK\r\n"
                        "Server: hsoap/2.8\r\n"
                        "Content-Type: text/html;charset=utf-8\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n\r\n",
                        len);
	memcpy(p_bufs+tlen, readBuf, len);
    tlen += len;
	send(fileno(fp),p_bufs,tlen,0);
	free(readBuf);
	free(p_bufs);
	
	fflush(fp);
	printf("send data end\n");
}

void do_cat(char* arg,FILE *fp){
	int fd=fileno(fp);
    // header(fp,"text/plain");
	fprintf(fp,"HTTP/1.0 200 OK\r\n");
    fprintf(fp,"Content-type:text/plain");
    fprintf(fp,"\r\n\r\n");
    fflush(fp);

	printf("store origin STDOUT_FILENO\n");
	int savefd = dup(STDOUT_FILENO);
	printf("redirect towards server\n");
    dup2(fd,STDOUT_FILENO);
   
    //printf("do some work\n");
	pid_t sub=fork();
	if(sub==0){
		execl("/bin/cat","cat",arg,NULL);
	}
    
	dup2(savefd, STDOUT_FILENO);
    printf("redirect towards STDOUT_FILENO\n");
}

// void do_pic(char* arg,FILE *fp){
// 	printf("send data start: HTTP/1.0 200 OK\n");
	
// 	char buf[BUF_SIZE];

// 	FILE* send_file;
// 	send_file=fopen(arg, "rb");

// 	// char path[SMALL_BUF];
// 	// memset(path,0x00,sizeof(path));
// 	// getcwd(path,sizeof(path)-1);
// 	// char* p=path;
// 	// char* fff=strcat(p,"/");
// 	// char* ff=strcat(p,arg);
// 	// int* in_length = new int();
//     // char* in_buffer = read_image(ff, in_length);

// 	char protocol[]="HTTP/1.0 200 OK\r\n";
// 	char server[]="Server:Linux Web Server \r\n";
// 	char cnt_len[]="Content-length:2048\r\n";
// 	char cnt_type[]="Content-type:image/jpeg\r\n";
// 	// char length[SMALL_BUF];
// 	// sprintf(length, "Content-Length:%d\r\n\r\n", *in_length);

// 	// if(in_buffer==NULL)
// 	// {
// 	// 	send_error(fp);
// 	// 	return;
// 	// }
// 	if(send_file==NULL)
// 	{
// 		send_error(fp);
// 		return;
// 	}

// 	fputs(protocol, fp);
// 	fputs(server, fp);
// 	fputs(cnt_len, fp);
// 	fputs(cnt_type, fp);
// 	//fputs(length, fp);

// 	while(fgets(buf, BUF_SIZE, send_file)!=NULL) 
// 	{
// 		fputs(buf, fp);
// 		fflush(fp);
// 	}
// 	//fputs(in_buffer,fp);
// 	fflush(fp);
// 	//printf("file content:%s\n",in_buffer);
// 	printf("send data end\n");
// }

int get_char_times(char* str,char c){
   int times=0;
   int index=0;
   char cc=str[0];
   while(cc!='\0'){
      if(cc==c){
         times++;
      }
      cc=str[++index];
   }
   return times;
}

void strmcpy(char *p, char *a, int m) {
	int count = 0;
	int i = 0, j = m;
	while ((*p != '\0')&&(*(p+j) != '\0')) {
		*(a + i) = *(p + j);
		i++;
		j++;
	}
    *(a + i)='\0';
}

void get_time(char* time_str){
    time_t the_time;
    (void)time(&the_time);
    strcpy(time_str,ctime(&the_time));
}

void log_fun(FILE *log_fp,char *time_log,char *temp){
    log_fp=fopen("log.txt","a+");
    get_time(time_log);
    if(strlen(temp)!=0){
        fprintf(log_fp,"%s",time_log);
        fprintf(log_fp,"%s",temp);
        //memset(temp,0,sizeof(temp));
    }
    fclose(log_fp);
    log_fp=NULL;
}



int sendPic(FILE *f,char* arg){
    FILE * fp;
    int len;
    //char readBuf[600*1024] = {0}; //600k
	char* readBuf=(char*)malloc(1024*1024*2);
    	
    int tlen;
    char * p_bufs = NULL; 
        
    /* read data */
	fp = fopen(arg, "rb");
	if (NULL == fp)
	{
		return -1;
	}
	fseek(fp, 0, SEEK_END);

	len = ftell(fp);
	printf("len = %d\n",len);
	if (len <= 0)
	{
		fclose(fp);
		return -1;
	}
	fseek(fp, 0, SEEK_SET);
    printf("len = %d\n",len);

    len = fread(readBuf, 1, len, fp);
	printf("len = %d\n",len);
	printf("strlen = %d\n",strlen(readBuf));

	fclose(fp);
    
    /* send data */
    p_bufs = (char *)malloc(len + 1024);
    if (NULL == p_bufs)
    {
        printf("malloc error!\n");
        return -1;
    }
    
    tlen = sprintf(p_bufs,	"HTTP/1.1 200 OK\r\n"
                        "Server: hsoap/2.8\r\n"
                        "Content-Type: image/jpeg\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n\r\n",
                        len);

    memcpy(p_bufs+tlen, readBuf, len);
    tlen += len;

    send(fileno(f), p_bufs, tlen, 0);

    free(p_bufs);
	free(readBuf);
    return 0;
}

int make_server_socket(int portnum){
    int listenfd;//
    struct sockaddr_in serv_addr;
    int temp;

    bzero((void*)&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(portnum);

    if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1){
        return -1;
    }
    if(listen(listenfd,QUEUE_SIZE)!=0){
        return -1;
    }
    return listenfd; 
}

int connect_to_server(char* host,int portnum){
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *hp;

    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
        return -1;
    }

    bzero(&serv_addr,sizeof(serv_addr));
    if((hp=gethostbyname(host))==NULL){
        return -1;
    }
    bcopy(hp->h_addr,(struct sockaddr*)&serv_addr.sin_addr,hp->h_length);
    serv_addr.sin_port=htons(portnum);
    serv_addr.sin_family=AF_INET;

    if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))!=0){
        return -1;
    }
    return sockfd;
}