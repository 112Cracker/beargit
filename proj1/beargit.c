#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);
   
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");

  return 0;
}



/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * - read the file .beargit/.index
 * - print a line for each tracked file
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_status() {
  /* COMPLETE THE REST */
  FILE *findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  int count = 0;

  printf("%s\n", "Tracked files:");
  while(fgets(line, FILENAME_SIZE, findex)) {
    count ++;
    strtok(line, "\n");
    printf("<%s>\n", line);
  }
  printf("<%d> files total\n", count);
  fclose(findex);
  return 0;
}

/*
 * helper function for beargit_rm
 */
int is_same_filename(char *tracked_filename, const char *filename) {
  int len1 = strlen(tracked_filename);
  int len2 = strlen(filename);
  int i;
  if (len1 != len2) return 0;
  for (i = 0; i < len1; i++) {
    if (tracked_filename[i] != filename[i]) return 0;
  }
  return 1;
}

/*
 * helper function for beargit_rm
 */
int rm_line_from_index(int delete_line) {
  FILE *findex, *dupfile;
  int curr_line = 0;

  findex = fopen(".beargit/.index", "r");
  dupfile = fopen(".beargit/.dupfile", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, FILENAME_SIZE, findex)) {
    strtok(line, "\n");
    curr_line ++;
    if (curr_line != delete_line) {
      fputs(line, dupfile);
    }
  }
  fclose(findex);
  fclose(dupfile);
  remove(".beargit/.index");
  rename(".beargit/.dupfile", ".beargit/.index");

  return(0);
}

/* beargit rm <filename>
 * 
 * - remove the File <filename> in the .beargit/.index
 * - print an error if the filename passed is not being tracked
 *
 * Possible errors (to stderr):
 * >> Error: File <filename> is not being tracked
 *
 * Output(to stdout):
 * - None if successful
 */
int beargit_rm(const char *filename) {
  /* COMPLETE THE REST */
  FILE *findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  int delete_line = 0, isFound = 0;

  while(fgets(line, FILENAME_SIZE, findex)) {
    strtok(line, "\n");
    delete_line ++;
    if (is_same_filename(line, filename)) {
      isFound = 1;
      fclose(findex);
      rm_line_from_index(delete_line);
    }
  }

  if (!isFound) {
    fprintf(stderr, "The file <%s> is not being tracked\n", filename);
  }
  return 0;
}

/* beargit_commit -m <msg>
 * 
 */
const char* go_bears = "GO BEARS!";

/*
 * Helper function for is_commit_msg_ok
 */
int has_go_bears(char *msg_p, char *go_bears_p) {
  while (*go_bears_p != '\0') {
    if (*go_bears_p != *msg_p) {
      return(0);
    }
    msg_p ++;
    go_bears_p ++;
  }
  return(1);
}

/*
 * Check if the commit message include 'GO BEARS!'
 *
 * Output;
 * 1 if the commit message include 'GO BEARS'
 * 0 if not
 */
int is_commit_msg_ok(const char* msg) {
  /* COMPLETE THE REST */
  char *p1 = msg, *p2 = go_bears;

  while (*p1 != '\0') {
    if (*p1 == 'G' && has_go_bears(p1, p2)) {
      return(1);
    }
    p1 ++;
  }
  return 0;
}

/*
 * Generate unique commit id(tag id)
 */
void next_commit_id_part1(char* commit_id) {
  /* COMPLETE THE REST */
  char *p = commit_id;
  while(*p != '\0') {
    if (*p == '6') {
      *p = '1';
      break;
    } else if (*p == '1') {
      *p = 'c';
      break;
    } else if (*p == 'c') {
      *p = '6';
      p ++;
    } else if (*p == '0') {
      *p = '6';
      p ++;
      continue;
    } 
  }
} 

 /*
 * Check whether the commit string contains "GO BEARS!"
 * Generate the next ID(newid)
 * Generate a new directory .beargit/<newid>
 * Copy .beargit/.index, .beargit/.prev and all tracked files into .beargit/<newid>
 * Store the commit message <msg> into .beargit/<newid>/.msg
 * Write the new ID into .beargit/.prev
 * 
 * Possible errors(to stderr)
 * >> Error: Message must contain "GO BEARS!"
 *
 * Output(to stdout)
 * - None of successful
 *
 */
int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id); 
  /* COMPLETE THE REST */
  char *new_dir = malloc(strlen(".beargit/") + strlen(commit_id) + 1);
  sprintf(new_dir, "%s/%s", ".beargit", commit_id);
  fs_mkdir(new_dir);

  // copy .beargit/.index to .beargit/<newid>
  char *new_index = malloc(strlen(new_dir) + strlen("/.index") + 1);
  sprintf(new_index, "%s/%s", new_dir, ".index");
  fs_cp(".beargit/.index", new_index);

  // copy .beargit/.pre to .beargit/<newid>
  char *new_pre = malloc(strlen(new_dir) + strlen("/.prev") + 1);
  sprintf(new_pre, "%s/%s", new_dir, ".prev");
  fs_cp(".beargit/.prev", new_pre);

  // copy all tracked files to .beagit/<newid>
  FILE *pre = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, FILENAME_SIZE, pre)) {
    strtok(line, "\n");
    char *new_file_dir = malloc(strlen(new_dir) + strlen("/")+ strlen(line) + 1);
    sprintf(new_file_dir, "%s/%s", new_dir, line);
    fs_cp(line, new_file_dir);
  }
  fclose(pre); 

  // write the new ID into .beargit/.prev
  write_string_to_file(".beargit/.prev", commit_id);
  // store the commit message into .beargit/<newid>/.msg
  char *msg_dir = malloc(strlen(new_dir) + strlen("/.msg") + 1);
  sprintf(msg_dir, "%s/%s", new_dir, ".msg");
  write_string_to_file(msg_dir, msg);

  return 0;
}

/*
 * Check if the commit_id is the initial commit
 * 
 * Output:
 * return 1 if the commit_id is the initial commit
 * return 0 if not
 */
int is_no_commit(char *commit_id) {
  int i, len = strlen(commit_id);
  for (i = 0; i < len; i++) {
    if (commit_id[i] != '0') return 0;
  }
  return 1;
}

/* beargit log
 *
 * Print out either all or limit number of commits
 *
 * Possible errors (to stderr)
 * >> Error: There are no commits!
 *
 * Output(to stdout)
 * - None if successful 
 * 
 */
int beargit_log(int limit) {
  /* COMPLETE THE REST */
  char new_commit[COMMIT_ID_SIZE];
  char *commit_p = new_commit; // pointer to the current tracked commit
  read_string_from_file(".beargit/.prev", new_commit, COMMIT_ID_SIZE);
  int count = 0;

  if (is_no_commit(new_commit)) {
    fprintf(stderr, "%s\n", "There are no commits!");
  }

  while (!is_no_commit(commit_p) && count < limit) {
    // print commit<ID>
    printf("commit<%s>\n", commit_p);

    char *msg_dir = malloc(strlen(".beargit") + COMMIT_ID_SIZE + strlen("/.msg") + 1);
    sprintf(msg_dir, "%s/%s/%s", ".beargit", commit_p, ".msg");
    char msg[512];
    read_string_from_file(msg_dir, msg, 512);
    // print <msg>
    printf("\t<%s>\n", msg);

    // get last commit
    char last_commit[COMMIT_ID_SIZE];
    char *last_commit_dir = malloc(strlen(".beargit/") + COMMIT_ID_SIZE + 1);
    sprintf(last_commit_dir, "%s/%s/.prev", ".beargit", commit_p);
    read_string_from_file(last_commit_dir, last_commit, COMMIT_ID_SIZE);
    // update 
    commit_p = last_commit;
  }
  return 0;
}


const char* digits = "61c";

void next_commit_id(char* commit_id) {
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // The first COMMIT_ID_BRANCH_BYTES=10 characters of the commit ID will
  // be used to encode the current branch number. This is necessary to avoid
  // duplicate IDs in different branches, as they can have the same pre-
  // decessor (so next_commit_id has to depend on something else).
  int n = get_branch_number(current_branch);
  for (int i = 0; i < COMMIT_ID_BRANCH_BYTES; i++) {
    commit_id[i] = digits[n%3];
    n /= 3;
  }

  // Use next_commit_id to fill in the rest of the commit ID.
  next_commit_id_part1(commit_id + COMMIT_ID_BRANCH_BYTES); // pointer arithmatic
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the homework 1 spec.
 *
 */

int beargit_branch() {
  /* COMPLETE THE REST */
  char *curr_branch = malloc(BRANCHNAME_SIZE);
  read_string_from_file(".beargit/.current_branch", curr_branch, BRANCHNAME_SIZE);

  FILE *fbranches = fopen(".beargit/.branches", "r");
  char line[BRANCHNAME_SIZE];
  while(fgets(line, BRANCHNAME_SIZE, fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, curr_branch) == 0) {
      printf("*<%s>\n", line);
    } else {
      printf("<%s>\n", line);
    }
  }
  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the homework 1 spec.
 *
 */

int checkout_commit(const char* commit_id) {
  /* COMPLETE THE REST */
  FILE *findex = fopen(".beargit/.index", "w");
  char *commit_f_dir = malloc(strlen(".beargit/") + strlen(commit_id) + strlen("/.index") + 1);
  sprintf(commit_f_dir, "%s/%s/%s", ".beargit", commit_id, ".index");

  FILE *fcommit = fopen(commit_f_dir, "r")

  char line[FILENAME_SIZE];
  while (fgets(line, FILENAME_SIZE, fcommit)) {
    strtok(line, "\n");
    fprintf(findex, "%s\n", line);
  }
  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  /* COMPLETE THE REST */
  char new_commit[COMMIT_ID_SIZE];
  char *commit_p = new_commit; // pointer to the current tracked commit
  read_string_from_file(".beargit/.prev", new_commit, COMMIT_ID_SIZE);

  if (strcmp(commit_p, commit_id) == 0) {
    return 1;
  }

  while (!is_no_commit(commit_p)) {
    // get last commit
    char last_commit[COMMIT_ID_SIZE];
    char *last_commit_dir = malloc(strlen(".beargit/") + COMMIT_ID_SIZE + 1);
    sprintf(last_commit_dir, "%s/%s/.prev", ".beargit", commit_p);
    read_string_from_file(last_commit_dir, last_commit, COMMIT_ID_SIZE);

    if (strcmp(last_commit, commit_id) == 0) {
      return 1;
    }
    // update 
    commit_p = last_commit;
  }
  return 0;
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // If not detached, update the current branch by storing the current HEAD into that branch's file...
  // Even if we cancel later, this is still ok.
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

  // Check whether the argument is a commit ID. If yes, we just stay in detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    printf("%s\n", arg);
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    if (!fs_check_dir_exists(commit_dir)) {
      fprintf(stderr, "ERROR: Commit %s does not exist\n", arg);
      return 1;
    }

    // Set the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(branch_name) >= 0);

  // Check for errors.
  if (!(!branch_exists || !new_branch)) {
    fprintf(stderr, "ERROR: A branch named %s already exists\n", branch_name);
    return 1;
  } else if (!branch_exists && new_branch) {
    fprintf(stderr, "ERROR: No branch %s exists\n", branch_name);
    return 1;
  }

  // File for the branch we are changing into.
  char* branch_file = ".beargit/.branch_"; 
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file); 
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}
