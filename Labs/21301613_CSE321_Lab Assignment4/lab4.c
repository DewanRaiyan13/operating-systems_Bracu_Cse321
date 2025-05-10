#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_USERS 3
#define MAX_RESOURCES 3
#define MAX_NAME_LEN 20

typedef enum {
    READ = 1,
    WRITE = 2,
    EXECUTE = 4
} Permission;

// User and Resource Definitions
typedef struct {
    char name[MAX_NAME_LEN];
} User;

typedef struct {
    char name[MAX_NAME_LEN];
} Resource;

// ACL Entry
typedef struct {
    char userName[MAX_NAME_LEN];
    int permissions; // bitmask
} ACLEntry;

typedef struct {
    Resource resource;
    ACLEntry acl[MAX_USERS];
    int acl_count;
} ACLControlledResource;

// Capability Entry
typedef struct {
    char resourceName[MAX_NAME_LEN];
    int permissions; // bitmask
} Capability;

typedef struct {
    User user;
    Capability capabilities[MAX_RESOURCES];
    int cap_count;
} CapabilityUser;

// Utility Functions
void printPermissions(int perm) {
    int first = 1;
    if (perm & READ) {
        printf("READ");
        first = 0;
    }
    if (perm & WRITE) {
        if (!first) printf(", ");
        printf("WRITE");
        first = 0;
    }
    if (perm & EXECUTE) {
        if (!first) printf(", ");
        printf("EXECUTE");
    }
    if (perm == 0) {
        printf("None");
    }
}

int hasPermission(int userPerm, int requiredPerm) {
    return (userPerm & requiredPerm) == requiredPerm;
}

// ACL System
void checkACLAccess(ACLControlledResource *res, const char *userName, int perm) {
    int found = 0;
    for (int i = 0; i < res->acl_count; i++) {
        if (strcmp(res->acl[i].userName, userName) == 0) {
            found = 1;
            printf("ACL Check: User %s requests ", userName);
            printPermissions(perm);
            printf(" on %s: ", res->resource.name);
            if (hasPermission(res->acl[i].permissions, perm)) {
                printf("Access GRANTED\n");
            } else {
                printf("Access DENIED\n");
            }
            break;
        }
    }
    if (!found) {
        printf("ACL Check: User %s has NO entry for resource %s: Access DENIED\n", userName, res->resource.name);
    }
}

// Capability System
void checkCapabilityAccess(CapabilityUser *user, const char *resourceName, int perm) {
    int found = 0;
    for (int i = 0; i < user->cap_count; i++) {
        if (strcmp(user->capabilities[i].resourceName, resourceName) == 0) {
            found = 1;
            printf("Capability Check: User %s requests ", user->user.name);
            printPermissions(perm);
            printf(" on %s: ", resourceName);
            if (hasPermission(user->capabilities[i].permissions, perm)) {
                printf("Access GRANTED\n");
            } else {
                printf("Access DENIED\n");
            }
            break;
        }
    }
    if (!found) {
        printf("Capability Check: User %s has NO capability for %s: Access DENIED\n", user->user.name, resourceName);
    }
}

int main() {
    // Create users and resources
    User users[MAX_USERS] = {{"Alice"}, {"Bob"}, {"Charlie"}};
    Resource resources[MAX_RESOURCES] = {{"File1"}, {"File2"}, {"File3"}};

    // ACL Setup
    ACLControlledResource acl_res[MAX_RESOURCES];
    for (int i = 0; i < MAX_RESOURCES; i++) {
        acl_res[i].resource = resources[i];
        acl_res[i].acl_count = 0;
    }

    // Sample ACL entries
    acl_res[0].acl[0] = (ACLEntry){"Alice", READ | WRITE};
    acl_res[0].acl[1] = (ACLEntry){"Bob", READ};
    acl_res[0].acl_count = 2;
    acl_res[1].acl[0] = (ACLEntry){"Alice", READ};
    acl_res[1].acl_count = 1;
    acl_res[2].acl_count = 0;

    // Capability Setup
    CapabilityUser cap_user[MAX_USERS];
    for (int i = 0; i < MAX_USERS; i++) {
        cap_user[i].user = users[i];
        cap_user[i].cap_count = 0;
    }

    // User capabilities
    cap_user[0].capabilities[0] = (Capability){"File1", READ | WRITE};
    cap_user[0].cap_count = 1;
    cap_user[1].capabilities[0] = (Capability){"File1", READ};
    cap_user[1].cap_count = 1;
    cap_user[2].cap_count = 0;

    // Test ACL
    checkACLAccess(&acl_res[0], "Alice", READ);
    checkACLAccess(&acl_res[0], "Bob", WRITE);
    checkACLAccess(&acl_res[0], "Charlie", READ);

    // Capability checks
    checkCapabilityAccess(&cap_user[0], "File1", WRITE);
    checkCapabilityAccess(&cap_user[1], "File1", WRITE);
    checkCapabilityAccess(&cap_user[2], "File2", READ);

    return 0;
}
