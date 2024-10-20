#include "./login/login.h"
#include "./sessions/sessions.h"
#include "./controller/admin/adminController.h"
#include "./controller/employee/employeeController.h"
#include "./controller/customer/customerController.h"
#include "./controller/manager/mgrController.h"
#include "./admin/admin.h"
#include "./service/employee/employeeService.h"
#include "./service/customer/customerService.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT 6053
#define MAX_BUFFER 1024

#define BUFFER_SIZE 512

void writeMainMsg(int sd, const char* msg) {
    write(sd, msg, strlen(msg));
}

void getInputMain(int sd, char *input, size_t size) {
    ssize_t bytesRead = read(sd, input, size);
        input[bytesRead - 1] = '\0';  
        if (input[bytesRead - 2] == '\r') {
        input[bytesRead - 2] = '\0';
    }
}
void user_login(int sd) {

    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char wbuffer[BUFFER_SIZE];
    long userId;

    writeMainMsg(sd, "Enter your username: ");
    getInputMain(sd, username, MAX_USERNAME_LEN);
    
    userId = atol(username);
    writeMainMsg(sd,"Enter your password: ");
     getInputMain(sd, password, MAX_PASSWORD_LEN);
    CustomerDTO currCust;
    memset(&currCust, 0, sizeof(CustomerDTO));

    EmployeeDTO currEmp;
    memset(&currEmp, 0, sizeof(EmployeeDTO));
    
    int type = 0;

    if((get_customer_data(userId, &currCust) == 0 && (strncmp(currCust.status, CUST_STATUS_ACTIVE, sizeof(CUST_STATUS_ACTIVE)) == 0)) ) {
        type = 1;
        
    } else if (get_employee_data(userId, &currEmp) == 0 && (strncmp(currEmp.status, EMP_STATUS_ACTIVE, sizeof(EMP_STATUS_ACTIVE)) == 0)) {
        if(strncmp(currEmp.role, EMP_ROLE_EMP, sizeof(EMP_ROLE_EMP)) == 0) 
            type =2;
        else if(strncmp(currEmp.role, EMP_ROLE_MGR, sizeof(EMP_ROLE_MGR)) == 0)
            type = 3;
    }

    if(type == 0) {
        writeMainMsg(sd, "Error: Invalid Username\n");
        return;
    }

    if (authenticate_user(username, password)) {

        if(create__new_session(username, 0, getpid())>0) {
            if(type == 1) {
                snprintf(wbuffer, BUFFER_SIZE, "Welcome: %s", currCust.name);
                write(sd, wbuffer, strlen(wbuffer));
                customer_session(sd, userId);
            } else if( type == 2) {
                snprintf(wbuffer, BUFFER_SIZE, "Welcome: %s", currEmp.name);
                write(sd, wbuffer, strlen(wbuffer));
                employee_session(sd, userId);
            }
            else if( type == 3) {
                snprintf(wbuffer, BUFFER_SIZE, "Welcome: %s", currEmp.name);
                write(sd, wbuffer, strlen(wbuffer));
                mgr_session(sd, userId);
            }
                
        }
    } else {
        writeMainMsg(sd, "Incorrect username password.\n");
    }
}

void admin_login(int sd) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char wbuffer[BUFFER_SIZE];
    bzero(username, MAX_USERNAME_LEN);
    writeMainMsg(sd, "Enter your username: ");
    ssize_t bytesRead;
     getInputMain(sd, username, MAX_USERNAME_LEN);
    
    writeMainMsg(sd, "Enter your password: ");
    getInputMain(sd, password, MAX_PASSWORD_LEN);
    
    AdminDTO currAdmin;
    memset(&currAdmin, 0, sizeof(AdminDTO));

    if(getAdmin(username, &currAdmin) < 0 || (strcmp(currAdmin.status, CUST_STATUS_ACTIVE) != 0)  ) {
        writeMainMsg(sd, "Error Invalid username \n");
        return;
    }

    if (authenticate_user(username, password)) {

        if(create__new_session(username, 0, getpid())>0) {
                snprintf(wbuffer, BUFFER_SIZE, "Welcome: %s", currAdmin.name);
                write(sd, wbuffer, strlen(wbuffer));
                admin_sess(sd, username);
        }
    } else {
        writeMainMsg(sd, "Incorrect username password.\n");
    }
}


void bank_main_menu(int sd) {
    char inputbuffer[BUFFER_SIZE];
    ssize_t bytesRead;
    int mop;
    while (1) {
        // Show the welcome menu using the writeMainMsg function
        writeMainMsg(sd, "\n--- Welcome to The Grand Old Bank ---\n");
        writeMainMsg(sd, "1. Login\n");
        writeMainMsg(sd, "2. Admin Login\n");
        writeMainMsg(sd, "3. Exit\n");
        writeMainMsg(sd, "Enter your choice: ");

        // Read user input
        getInputMain(sd, inputbuffer, BUFFER_SIZE);
         // Remove newline if needed
            mop = atoi(inputbuffer);
            switch (mop) {
                case 1:
                    user_login(sd);
                    break;
                case 2:
                    admin_login(sd);
                    break;
                case 3:
                    writeMainMsg(sd, "Exiting...\n");
                    return;  // Exit the infinite loop
                default:
                    writeMainMsg(sd, "Invalid option, please try again.\n");
                    break;
            
        }
    }
}



void handle_client(int client_sock) {
    char buffer[MAX_BUFFER];
    int n;
    int pid = getpid();
    // Communicate with the client
    bank_main_menu(client_sock);
    
    close(client_sock);
    printf("Client disconnected.\n");
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

   
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("accept failed");
            continue;
        }

        printf("Client connected!\n");

        // child process to handle the client
        if (fork() == 0) {
            close(server_sock); 
            handle_client(client_sock);
            exit(0);  
        }

        close(client_sock);  
    }

    close(server_sock);
    return 0;
}
