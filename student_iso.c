#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 100
#define STUDENT_INFORMATION "Student_information.txt"
#define MAX_NAME_LEN 100
#define MAX_MAJOR_LEN 100

// 学生结构体
typedef struct Student {
    char name[MAX_NAME_LEN];
    int stuid;
    int age;
    char major[MAX_MAJOR_LEN];
    struct Student *next;
} Student;

// 学生数据库结构体
typedef struct {
    Student *buckets[HASH_SIZE];
    int count;
} StudentDB;

// 函数声明
StudentDB* init_system();
void free_database(StudentDB *db);
void Add_Stu(StudentDB *db, Student *stu);
void save_students_to_file(StudentDB *db);
void Load_From_File(StudentDB *db, const char *filename);
Student* Search_query_in_memory(StudentDB *db, int stuid);
void Show_All_Stu(StudentDB *db);
void Remove_Student(StudentDB *db, int stuid);
void Modify_Stu(StudentDB *db, int stuid);
void clear_input_buffer();
void display_menu();

int main() {
    StudentDB *db = init_system();
    if (db == NULL) {
        fprintf(stderr, "系统初始化失败\n");
        return 1;
    }

    Load_From_File(db, STUDENT_INFORMATION);
    printf("系统初始化完成，已加载 %d 名学生\n", db->count);

    int choice;
    do {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("输入无效，请重新选择\n");
            continue;
        }
        clear_input_buffer();

        switch(choice) {
            case 1: { // 添加学生
                Student *new_stu = malloc(sizeof(Student));
                if (!new_stu) {
                    printf("内存分配失败\n");
                    break;
                }

                printf("请输入学号: ");
                scanf("%d", &new_stu->stuid);
                clear_input_buffer();

                printf("请输入姓名: ");
                fgets(new_stu->name, sizeof(new_stu->name), stdin);
                new_stu->name[strcspn(new_stu->name, "\n")] = '\0';

                printf("请输入年龄: ");
                scanf("%d", &new_stu->age);
                clear_input_buffer();

                printf("请输入专业: ");
                fgets(new_stu->major, sizeof(new_stu->major), stdin);
                new_stu->major[strcspn(new_stu->major, "\n")] = '\0';

                Add_Stu(db, new_stu);
                save_students_to_file(db);
                break;
            }
            case 2: // 显示所有学生
                Show_All_Stu(db);
                break;
            case 3: { // 查询学生
                int stuid;
                printf("请输入要查询的学号: ");
                scanf("%d", &stuid);
                clear_input_buffer();

                Student *stu = Search_query_in_memory(db, stuid);
                if (stu) {
                    printf("\n查询结果:\n");
                    printf("学号: %d\n", stu->stuid);
                    printf("姓名: %s\n", stu->name);
                    printf("年龄: %d\n", stu->age);
                    printf("专业: %s\n", stu->major);
                } else {
                    printf("未找到学号为 %d 的学生\n", stuid);
                }
                break;
            }
            case 4: { // 删除学生
                int stuid;
                printf("请输入要删除的学号: ");
                scanf("%d", &stuid);
                clear_input_buffer();
                Remove_Student(db, stuid);
                break;
            }
            case 5: { // 修改学生
                int stuid;
                printf("请输入要修改的学号: ");
                scanf("%d", &stuid);
                clear_input_buffer();
                Modify_Stu(db, stuid);
                break;
            }
            case 6: // 退出系统
                printf("感谢使用学生管理系统，再见！\n");
                break;
            default:
                printf("无效选择，请重新输入！\n");
        }
    } while (choice != 6);

    save_students_to_file(db);
    free_database(db);
    return 0;
}

// 初始化学生数据库
StudentDB* init_system() {
    StudentDB *db = malloc(sizeof(StudentDB));
    if (db == NULL) {
        perror("内存分配失败");
        return NULL;
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        db->buckets[i] = NULL;
    }
    db->count = 0;
    return db;
}

// 释放数据库内存
void free_database(StudentDB *db) {
    if (db == NULL) return;

    for (int i = 0; i < HASH_SIZE; i++) {
        Student *current = db->buckets[i];
        while (current != NULL) {
            Student *next = current->next;
            free(current);
            current = next;
        }
    }
    free(db);
}

//输入学生信息到文件里面
/*
 *思路：我们输入学生信息，然后将其先保存到哈希表中，当我们本次的输入结束之后，我们再将他保存到文件中。
 *如果后续还有新的学生的话，我们直接再文件末尾做添加就可以了。
 */
void Add_Stu(StudentDB *db, Student *stu) {
    int hash = stu->stuid % HASH_SIZE;

    // 桶内头插法（修复：不需要判断是否为NULL）
    stu->next = db->buckets[hash];
    db->buckets[hash] = stu;

    db->count++;
    printf("学生 %d 添加成功！\n", stu->stuid);
}

// 保存学生信息到文件
void save_students_to_file(StudentDB *db) {
    FILE *file = fopen(STUDENT_INFORMATION, "w");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        Student *current = db->buckets[i];
        while (current != NULL) {
            fprintf(file, "%d,%s,%d,%s\n",
                   current->stuid, current->name,
                   current->age, current->major);
            current = current->next;
        }
    }
    fclose(file);
}

// 从文件加载学生信息
void Load_From_File(StudentDB *db, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Student *newStudent = malloc(sizeof(Student));
        if (sscanf(line, "%d,%[^,],%d,%[^,\n]",
                  &newStudent->stuid, newStudent->name,
                  &newStudent->age, newStudent->major) == 4) {
            Add_Stu(db, newStudent);
        } else {
            free(newStudent);
        }
    }
    fclose(file);
}

// 在内存中查找学生
Student* Search_query_in_memory(StudentDB *db, int stuid) {
    int hash = stuid % HASH_SIZE;
    Student *current = db->buckets[hash];
    while (current != NULL) {
        if (current->stuid == stuid) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// 显示所有学生信息
void Show_All_Stu(StudentDB *db) {
    if (db->count == 0) {
        printf("当前没有学生记录\n");
        return;
    }

    printf("\n当前所有学生信息:\n");
    printf("学号\t姓名\t年龄\t专业\n");
    printf("--------------------------------\n");

    for (int i = 0; i < HASH_SIZE; i++) {
        Student *current = db->buckets[i];
        while (current != NULL) {
            printf("%d\t%s\t%d\t%s\n",
                  current->stuid, current->name,
                  current->age, current->major);
            current = current->next;
        }
    }
}

// 从内存中删除学生
int Remove_Student_From_Memory(StudentDB *db, int stuid) {
    int hash = stuid % HASH_SIZE;
    Student **ptr = &db->buckets[hash];

    while (*ptr != NULL) {
        if ((*ptr)->stuid == stuid) {
            Student *to_delete = *ptr;
            *ptr = (*ptr)->next;
            free(to_delete);
            db->count--;
            return 1;
        }
        ptr = &(*ptr)->next;
    }
    return 0;
}

// 删除学生
void Remove_Student(StudentDB *db, int stuid) {
    if (Remove_Student_From_Memory(db, stuid)) {
        save_students_to_file(db);
        printf("学生 %d 已删除\n", stuid);
    } else {
        printf("未找到学号为 %d 的学生\n", stuid);
    }
}

// 修改学生信息
void Modify_Stu(StudentDB *db, int stuid) {
    Student *stu = Search_query_in_memory(db, stuid);
    if (stu == NULL) {
        printf("未找到学号为 %d 的学生\n", stuid);
        return;
    }

    printf("\n当前学生信息:\n");
    printf("学号: %d\n", stu->stuid);
    printf("姓名: %s\n", stu->name);
    printf("年龄: %d\n", stu->age);
    printf("专业: %s\n", stu->major);

    char input[MAX_NAME_LEN];
    printf("\n输入新信息(直接回车保留原值):\n");

    printf("新姓名 [%s]: ", stu->name);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        input[strcspn(input, "\n")] = '\0';
        strncpy(stu->name, input, sizeof(stu->name));
    }

    printf("新年龄 [%d]: ", stu->age);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        stu->age = atoi(input);
    }

    printf("新专业 [%s]: ", stu->major);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        input[strcspn(input, "\n")] = '\0';
        strncpy(stu->major, input, sizeof(stu->major));
    }

    save_students_to_file(db);
    printf("\n学生信息更新成功!\n");
}

// 清除输入缓冲区
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 显示菜单
void display_menu() {
    printf("\n学生管理系统\n");
    printf("1. 添加学生\n");
    printf("2. 显示所有学生\n");
    printf("3. 查询学生\n");
    printf("4. 删除学生\n");
    printf("5. 修改学生信息\n");
    printf("6. 退出\n");
    printf("请选择操作: ");
}