#include "types.h"
#include "user.h"
#include "login.h"
#include "crypto.h"
#include "fcntl.h"

struct User {
  int uid;
  uchar username[MAX_INPUT_SIZE + 1];
  uchar password[MAX_INPUT_SIZE + 1];
};

struct test {
    char name;
    int number[];
};

int max_uid = 0;
User* user_array;

/**
 * Hook into user/src/login/login_init.c in order to intialize any files or
 * data structures necessary for the login system
 * 
 * Called once per boot of xv6
 */
void init_hook() {

  int fd;

  fd = open("/etc/pw", O_CREATE | O_RDWR);
  if(fd >= 0) {
    //Need the size of the user_array before we read. How?
    if(read(fd, &user_array, sizeof(user_array)) != size){
        printf(1, "error: read from backup file failed\n");
        exit();
    }
    while (*user_array != 0) {
      max_uid++;
      user_array += sizeof(User);
    }
      //printf(1, "ok: create backup file succeed\n");
  } else {
      //printf(1, "error: create backup file failed\n");
      exit();
  }
  close(fd);

  create_user("root", "admin");







/*
  struct test t;
  t.name = 's';
  t.number[0] = 5;
  int f = 4;
  struct test r;

  fd = open("backup", O_CREATE | O_RDWR);
  if(fd >= 0) {
      printf(1, "ok: create backup file succeed\n");
  } else {
      printf(1, "error: create backup file failed\n");
      exit();
  }

  int size = sizeof(t);
  if(write(fd, &t, size) != size){
      printf(1, "error: write to backup file failed\n");
      exit();
  }
  printf(1, "write ok\n");
  close(fd);

  //////

  fd = open("backup", O_RDONLY);
  if(fd >= 0) {
      printf(1, "ok: read backup file succeed\n");
  } else {
      printf(1, "error: read backup file failed\n");
      exit();
  }

  if(read(fd, &r, size) != size){
      printf(1, "error: read from backup file failed\n");
      exit();
  }
  printf(1, "file contents number %d, %d\n", r.number[0], r.number[1]);
  printf(1, "read ok\n");
  close(fd);

  //////////////////////////////////

  t.number[1] = f;
  fd = open("backup", O_RDWR);
  if(fd >= 0) {
      printf(1, "ok: create backup file succeed\n");
  } else {
      printf(1, "error: create backup file failed\n");
      exit();
  }

  if(write(fd, &t, size) != size){
      printf(1, "error: write to backup file failed\n");
      exit();
  }
  printf(1, "write ok\n");
  close(fd);

  //////

  fd = open("backup", O_RDONLY);
  if(fd >= 0) {
      printf(1, "ok: read backup file succeed\n");
  } else {
      printf(1, "error: read backup file failed\n");
      exit();
  }

  if(read(fd, &r, size) != size){
      printf(1, "error: read from backup file failed\n");
      exit();
  }
  printf(1, "file contents number %d, %d\n", r.number[0], r.number[1]);
  printf(1, "read ok\n");
  close(fd);
*/

  // // Create the file with read and write permissions for root user
  // printf(1, "calling init_hook\n");
  // //open("/etc/passwd", O_CREATE|O_RDWR);
  // //create_user("root", "admin");
}


/**
 * Check if user exists in system 
 * 
 * @param username A null-terminated string representing the username
 * @return 0 on success if user exists, -1 for failure otherwise
 */
int does_user_exist(char *username) {
  while (*user_array != 0) {
    if (strcmp((const char*)user_array->username, (const char*)username) == 0) {
      return 0;
    }
    user_array += sizeof(User);
  }
  return -1;
}

/**
 * Create a user in the system associated with the username and password. Cannot
 * overwrite an existing username with a new password. Expectation is for
 * created users to have a unique non-root uid.
 * 
 * @param username A null-terminated string representing the username
 * @param password A null-terminated string representing the password
 * @return 0 on success, -1 for failure
 */
int create_user(char *username, char *password) {
  if(strlen(username) > MAX_INPUT_SIZE || strlen(password) > MAX_INPUT_SIZE)
    return -1;

  if(does_user_exist(username) == 0)
    return -1;

  struct User user;
  strcpy((char*) user.username, (char*) username);

  // Very secure salt
  char *salt = "hifghidsyfgdfvgalsdicbvdalsycvglsuygvceiyfgvluy";
  char *hashed_salt[SHA256_SIZE_BYTES];
  sha256(salt, strlen(salt), (uchar*) hashed_salt);

  char *hashed_password[SHA256_SIZE_BYTES];
  sha256(password, strlen(password), (uchar*) hashed_password);

  char *password_entry[strlen((const char*)hashed_salt) + strlen((const char*)hashed_password)];

  strcpy((char*)password_entry, (const char*)hashed_salt);
  password_entry[10] = '\0';

  strcpy((char*)password_entry + strlen((const char*)password_entry), (const char*)hashed_password);
  strcpy((char*)password_entry + strlen((const char*)password_entry), (const char*)(hashed_salt + 10));
  strcpy((char*)user.password, (const char*)password_entry);

  user.uid = getuid();
  
  int passwordFD = open("/etc/passwd", O_RDWR);
  write(passwordFD, &user, sizeof(User));
  close(passwordFD);
  return 0;
}

/**
 * Login a user in the system associated with the username and password. Launch
 * the shell under the right permissions for the user. If no such user exists
 * or the password is incorrect, then login will fail.
 * 
 * @param username A null-terminated string representing the username
 * @param password A null-terminated string representing the password
 * @return no return on success, -1 for failure
 */
int login_user(char *username, char *password) {
  return -1;
}
