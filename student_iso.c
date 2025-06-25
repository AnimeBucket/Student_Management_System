#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 100
#define STUDENT_INFORMATION "Student_information.txt"
#define MAX_NAME_LEN 100
#define MAX_MAJOR_LEN 100

// ѧ���ṹ��
typedef struct Student {
    char name[MAX_NAME_LEN];
    int stuid;
    int age;
    char major[MAX_MAJOR_LEN];
    struct Student *next;
} Student;

// ѧ�����ݿ�ṹ��
typedef struct {
    Student *buckets[HASH_SIZE];
    int count;
} StudentDB;

// ��������
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
        fprintf(stderr, "ϵͳ��ʼ��ʧ��\n");
        return 1;
    }

    Load_From_File(db, STUDENT_INFORMATION);
    printf("ϵͳ��ʼ����ɣ��Ѽ��� %d ��ѧ��\n", db->count);

    int choice;
    do {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("������Ч��������ѡ��\n");
            continue;
        }
        clear_input_buffer();

        switch(choice) {
            case 1: { // ���ѧ��
                Student *new_stu = malloc(sizeof(Student));
                if (!new_stu) {
                    printf("�ڴ����ʧ��\n");
                    break;
                }

                printf("������ѧ��: ");
                scanf("%d", &new_stu->stuid);
                clear_input_buffer();

                printf("����������: ");
                fgets(new_stu->name, sizeof(new_stu->name), stdin);
                new_stu->name[strcspn(new_stu->name, "\n")] = '\0';

                printf("����������: ");
                scanf("%d", &new_stu->age);
                clear_input_buffer();

                printf("������רҵ: ");
                fgets(new_stu->major, sizeof(new_stu->major), stdin);
                new_stu->major[strcspn(new_stu->major, "\n")] = '\0';

                Add_Stu(db, new_stu);
                save_students_to_file(db);
                break;
            }
            case 2: // ��ʾ����ѧ��
                Show_All_Stu(db);
                break;
            case 3: { // ��ѯѧ��
                int stuid;
                printf("������Ҫ��ѯ��ѧ��: ");
                scanf("%d", &stuid);
                clear_input_buffer();

                Student *stu = Search_query_in_memory(db, stuid);
                if (stu) {
                    printf("\n��ѯ���:\n");
                    printf("ѧ��: %d\n", stu->stuid);
                    printf("����: %s\n", stu->name);
                    printf("����: %d\n", stu->age);
                    printf("רҵ: %s\n", stu->major);
                } else {
                    printf("δ�ҵ�ѧ��Ϊ %d ��ѧ��\n", stuid);
                }
                break;
            }
            case 4: { // ɾ��ѧ��
                int stuid;
                printf("������Ҫɾ����ѧ��: ");
                scanf("%d", &stuid);
                clear_input_buffer();
                Remove_Student(db, stuid);
                break;
            }
            case 5: { // �޸�ѧ��
                int stuid;
                printf("������Ҫ�޸ĵ�ѧ��: ");
                scanf("%d", &stuid);
                clear_input_buffer();
                Modify_Stu(db, stuid);
                break;
            }
            case 6: // �˳�ϵͳ
                printf("��лʹ��ѧ������ϵͳ���ټ���\n");
                break;
            default:
                printf("��Чѡ�����������룡\n");
        }
    } while (choice != 6);

    save_students_to_file(db);
    free_database(db);
    return 0;
}

// ��ʼ��ѧ�����ݿ�
StudentDB* init_system() {
    StudentDB *db = malloc(sizeof(StudentDB));
    if (db == NULL) {
        perror("�ڴ����ʧ��");
        return NULL;
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        db->buckets[i] = NULL;
    }
    db->count = 0;
    return db;
}

// �ͷ����ݿ��ڴ�
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

//����ѧ����Ϣ���ļ�����
/*
 *˼·����������ѧ����Ϣ��Ȼ�����ȱ��浽��ϣ���У������Ǳ��ε��������֮�������ٽ������浽�ļ��С�
 *������������µ�ѧ���Ļ�������ֱ�����ļ�ĩβ����ӾͿ����ˡ�
 */
void Add_Stu(StudentDB *db, Student *stu) {
    int hash = stu->stuid % HASH_SIZE;

    // Ͱ��ͷ�巨���޸�������Ҫ�ж��Ƿ�ΪNULL��
    stu->next = db->buckets[hash];
    db->buckets[hash] = stu;

    db->count++;
    printf("ѧ�� %d ��ӳɹ���\n", stu->stuid);
}

// ����ѧ����Ϣ���ļ�
void save_students_to_file(StudentDB *db) {
    FILE *file = fopen(STUDENT_INFORMATION, "w");
    if (file == NULL) {
        perror("�޷����ļ�");
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

// ���ļ�����ѧ����Ϣ
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

// ���ڴ��в���ѧ��
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

// ��ʾ����ѧ����Ϣ
void Show_All_Stu(StudentDB *db) {
    if (db->count == 0) {
        printf("��ǰû��ѧ����¼\n");
        return;
    }

    printf("\n��ǰ����ѧ����Ϣ:\n");
    printf("ѧ��\t����\t����\tרҵ\n");
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

// ���ڴ���ɾ��ѧ��
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

// ɾ��ѧ��
void Remove_Student(StudentDB *db, int stuid) {
    if (Remove_Student_From_Memory(db, stuid)) {
        save_students_to_file(db);
        printf("ѧ�� %d ��ɾ��\n", stuid);
    } else {
        printf("δ�ҵ�ѧ��Ϊ %d ��ѧ��\n", stuid);
    }
}

// �޸�ѧ����Ϣ
void Modify_Stu(StudentDB *db, int stuid) {
    Student *stu = Search_query_in_memory(db, stuid);
    if (stu == NULL) {
        printf("δ�ҵ�ѧ��Ϊ %d ��ѧ��\n", stuid);
        return;
    }

    printf("\n��ǰѧ����Ϣ:\n");
    printf("ѧ��: %d\n", stu->stuid);
    printf("����: %s\n", stu->name);
    printf("����: %d\n", stu->age);
    printf("רҵ: %s\n", stu->major);

    char input[MAX_NAME_LEN];
    printf("\n��������Ϣ(ֱ�ӻس�����ԭֵ):\n");

    printf("������ [%s]: ", stu->name);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        input[strcspn(input, "\n")] = '\0';
        strncpy(stu->name, input, sizeof(stu->name));
    }

    printf("������ [%d]: ", stu->age);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        stu->age = atoi(input);
    }

    printf("��רҵ [%s]: ", stu->major);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        input[strcspn(input, "\n")] = '\0';
        strncpy(stu->major, input, sizeof(stu->major));
    }

    save_students_to_file(db);
    printf("\nѧ����Ϣ���³ɹ�!\n");
}

// ������뻺����
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ��ʾ�˵�
void display_menu() {
    printf("\nѧ������ϵͳ\n");
    printf("1. ���ѧ��\n");
    printf("2. ��ʾ����ѧ��\n");
    printf("3. ��ѯѧ��\n");
    printf("4. ɾ��ѧ��\n");
    printf("5. �޸�ѧ����Ϣ\n");
    printf("6. �˳�\n");
    printf("��ѡ�����: ");
}