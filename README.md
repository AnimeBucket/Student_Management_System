# 前言：

这个是我写的一个学生管理系统的一个简介，这个系统的主要功能如下：

1. 添加学生信息
2. 删除学生信息
3. 显示所有学生信息
4. 查询学生信息
5. 修改学生信息

用户可以通过输入的序号来使用对应的功能。



# 系统框架设计：


![画板](https://cdn.nlark.com/yuque/0/2025/jpeg/29467292/1750758427116-aee07549-4d12-446d-a3cc-875e8ede8ef1.jpeg)

# 核心数据结构

## 学生结构体

```javascript
typedef struct Student {
    char name[100];         // 姓名
    int stuid;              // 学号(唯一标识)
    int age;                // 年龄
    char major[100];        // 专业
    struct Student *next;   // 下一个节点指针
} Student;
```

设计思路：

+ 作为系统的基本数据单元
+ 包含`next`指针形成链表结构
+ `stuid`作为唯一标识符(键值)
+ 使用固定大小数组简化内存管理

## 学生哈希表

```javascript
//学生数据库结构体
typedef struct {
    Student *table[HASH_SIZE];       //哈希桶数组
    Student *head;      // 链表头指针
    int count;				//学生人数
} StudentDB;
```

设计思路：

+ 使用链地址法解决哈希冲突
+ `table`数组存储每个桶的链表头
+ `head`指向全局链表头部，便于遍历
+ 固定大小哈希表简化实现

# 功能模块

## 初始化

```javascript
//初始化学生数据库
StudentDB* init_system() {
    StudentDB *db = malloc(sizeof(StudentDB));
    //判断内存是否分配成功
    if (db == NULL) {
        perror("malloc_error");
        return NULL;
    }

    //初始化hash_table
    for (int i =0; i < HASH_SIZE; i++) {
        db->table[i] = NULL;
    }

    //让链表的头指针为NULL
    db->head = NULL;

    //计算学生人数
    db->count = 0;

    return db;      //返回创建的数据库指针
}
```

我们跟踪看看效果：

![](https://cdn.nlark.com/yuque/0/2025/png/29467292/1750698317995-f75a42c2-f12b-4942-ad4a-58b4004b2588.png)

table 中的所有值也都变成了 NULL

这里的 StudentDB init_system() 中的 StudentDB*是<font style="color:#DF2A3F;">返回类型</font>，表示这个函数是一个<font style="color:#DF2A3F;">指向 StudentDB 结构体的指针</font>

init_system()是一个<font style="color:#DF2A3F;">函数名</font>

我们分配内存给数据库结构，然后设置合理的初始值都为 NULL，最后返回指针供其他函数使用。

## 添加学生信息

原先思路，将我们输入的每一个学生都添加到一个文件中，文件名为 Student_information.txt

然后代码流程思路为：

输入学生的学号，姓名，年龄和专业，然后通过哈希表来存入，当我们现在存完之后，通过一个文件队哈希表进行读取存放就可以达到本地保存了。但是再哈希表部分我们需要做一个判断就是我们这个学号是否是唯一的。也就是说，我们再添加学生这里我们得有两个函数，一个是输入学生信息到哈希表中，还有一个是保存到文件中

### Add_stu()

```c
void Add_Stu(StudentDB *db, Student *stu) {
    int hash = stu->stuid % HASH_SIZE;          //计算哈希值
    if (db->buckets[hash] == NULL) {            //如果哈希桶为NULL,也就表示已经初始化成功了，直接插入
        stu->next = db->buckets[hash];          //将新学生节点的 next 指针指向当前桶中的链表头节点
        db->buckets[hash] = stu;                //将新学生节点作为当前桶中链表的头节点
    }
}
```

在链式哈希表中，我们通常不会直接把 stu->stuid 和 stu->name 赋值给 db->key 和 db->val（正常的哈希表是key和val），因为每个桶实际上是一个链表的起点，链表中的每个节点都保存着一个键值对（在这里是 stuid 和其他学生信息）。链式哈希表通过为每个桶维护一个链表来解决哈希冲突，允许多个键值对存储在同一个桶中。

完整代码：

```c
//输入学生信息到文件里面
/*
 *思路：我们输入学生信息，然后将其先保存到哈希表中，当我们本次的输入结束之后，我们再将他保存到文件中。
 *如果后续还有新的学生的话，我们直接再文件末尾做添加就可以了。
 */
void Add_Stu(StudentDB *db, Student *stu) {
    int hash = stu->stuid % HASH_SIZE;
    
    // 桶内头插法（
    stu->next = db->buckets[hash];
    db->buckets[hash] = stu;
    
    db->count++;
    printf("学生 %d 添加成功！\n", stu->stuid);
}
```

问题：为什么我们的哈希表和链表是共享的同一份数据还是把同一份输入，保存在两个部分，一部分用于快速查询，一部分用于增删改？

> 学生信息是在内存中通过指针建立了两种不同的组织结构：一种是以哈希表为基础，用于快速查找；另一种是以全局链表为基础，用于整体遍历。这两种结构共享同一批学生数据节点，从不同角度组织这些数据，以满足不同场景下的操作需求。

现在我们可以来写第二部分的代码了，将学生信息保存到文件中，用于本地保存。

### save_students_to_file()

```c
//将学生信息保存到文件
void save_students_to_file(StudentDB *db) {
    FILE *file = fopen(STUDENT_INFORMATION, "w");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        Student *current = db->buckets[i];
        while (current != NULL) {
            fprintf(file, "%d,%s,%d,%s\n", current->stuid, current->name, current->age, current->major);
            current = current->next;
        }
    }

    fclose(file);
    printf("学生信息已保存到文件！\n");
}
```

## 查找学生信息

思路：查询模块应该有分为两种情况

  1. 我刚添加完数据，所有的数据都还在内存中，我可以直接通过哈希表来通过键值对查询 

 2. 我添加完了数据，但是我退出了程序，内存中已经没有原来的数据了，我需要从文件中读取源数据，然后再做查询，这个时候应该可以通过链表或者其他思路来做。

也就是说我们这个函数需要再写两个情况函数，query_in_memory()和Load_From_File()

### query_in_memory

我们先来写一下 query_in_memory 函数

```c
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
```

首先我们是计算哈希值，然后我们要从哈希表对应的桶中取出链表头指针，指针指向链表的第一个节点。然后我们需要判断我们当前的这个学号是否是我们要查询的学号，如果不是就跳到下一个节点。如果全部遍历完了都没有找到，则说明没有要查询的学号。

### search_student_in_file()

再来看看加载文件的思路是什么

首先我们要打开这个文件（fopen），然后判断我们这个文件是否是空的，非空的话我们就可以定义一个数组来保存每一行的学生信息了。我们将文件中的东西，逐行加载到哈希表中（fget），并且确保每次读取到的东西不超过缓存区的大小（避免缓冲区溢出）。创建一个实列用来保存数据（newstudent）并且动态分配内存，然后通过判断来看看我们输入的信息是否有 4 个信息（学号，名字，年龄，专业），满足则调用 Add_Stu()， 如果不满足则释放我们刚刚开辟的空间。最后读取完了就可以关闭这个文件了（fclose）

```c
//从文件加载学生信息到内存中的哈希表
void Load_From_File(StudentDB *db,const char *filename) {
    FILE *file = fopen("Student_information.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return ;
    }

    char line[256];                     //定义一个字符数组，用于存储每一行的内容
    while (fgets(line, sizeof(line), file)) {
        Student *newStudent = (Student *)malloc(sizeof(Student));
        if (sscanf(line, "%d,%[^,],%d,%[^,]", &newStudent->stuid, newStudent->name, &newStudent->age, newStudent->major) == 4)
        //这里的name和major是数组，他们本身就是地址，所以不需要取地址符
        {
            Add_Stu(db, newStudent);
        } else {
            free(newStudent);
            continue;
        }
    }
    fclose(file);
}
```

### Search_Stu()

ok 现在我们两个情况的代码都完成了，我们现在就可以来写最后的 search 模块了

模块思路：

因为用户添加完数据之后，可能习惯性直接就查询我们的用户有没有添加成功，所以可能添加完用户之后没有退出程序，我们就先看看能不能在内存中找到数据，没有的话在从文件中读取

```c
//综合查询
Student* Search_Stu(StudentDB *db, const char *filename, int stuid) {
    // 尝试在内存中的哈希表查找学生
    Student *foundStudent = Search_query_in_memory(db, stuid);

    if (foundStudent != NULL && foundStudent->stuid == stuid) {
        return foundStudent;
    } else if (foundStudent == NULL) {
        // 如果内存中没有找到学生，尝试从文件中加载学生信息到内存中
        Load_From_File(db, filename);
        // 再次从内存中查找学生
        foundStudent = Search_query_in_memory(db, stuid);
        if (foundStudent != NULL && foundStudent->stuid == stuid) {
            return foundStudent;
        }
    }

    return NULL;
}
```

## 删除学生信息

删除学生信息也是分两种情况，内存中也文件中。也是通过哈希表进行删除操作。

在内存中的时候，我们直接使用 hash 表来删除，然后保存到文件。

### remove_student_from_memory（）

首先我们看到我们的哈希表结构，我们使用的是链式地址法来解决冲突问题，所以我们的每一个桶就是一个链表。所以我们在内存中的删除思路就是：首先通过学号计算哈希值定位到桶，然后在该桶的链表中进行删除。

```c
int remove_student_from_memory(StudentDB *db, int stuid) {
    int hash = stuid % HASH_SIZE;
    Student *current = db->buckets[hash];
    Student *prev = NULL;
    
    while (current != NULL) {
        if (current->stuid == stuid) {
            // 从哈希桶链表中删除节点
            if (prev) {
                prev->next = current->next;
            } else {
                db->buckets[hash] = current->next;
            }
            
            free(current); // 释放内存
            db->count--;   // 更新计数
            return 1;
        }
        prev = current;
        current = current->next;
    }
    return 0;
}
```

我们首先还是计算哈希值，然后让 current 来获取哈希桶中的链表头节点，然后让 prev 来跟踪 current 的前一个链表节点，我的跟踪思路是一直循环遍历链表，直到为空，然后判断我们输入的学号是否匹配，匹配则让 prev 的 next 指向当前节点的 next（也就是 current->next) 然后我们在释放当前节点的内存空间以达到删除的效果。如果没有找到相对应的学号，我们就让 prev 来成为当前节点，current 移动到下一个节点的位置。

举一个实际例子来理解：

我有三个学号，101，102，103，我要删除 102，那代码第一次迭代的时候 current 指向的是 101，学号不匹配，则 prev 指向学号 101，然后让 current 来指向学好 102，第二次迭代，学号相匹配，然后就开始做删除操作。

`prev` 指针在遍历过程中始终跟随 `current` 指针，确保在找到目标节点时能够正确调整链表的指针结构。

### remove_student_from_file(

由于我们之前的铺垫，我们的这个部分可以写的比较轻松了哈哈哈，按照最开始的删除思路，现在内存中查找，没有就加载文件到内存，然后再一次从内存做查找删除。

```c
// 删除学生信息（从文件）
void remove_student_from_file(StudentDB *db, int stuid) {
    // 如果内存中没有数据，先从文件加载数据
    if (db->count == 0) {
        Load_From_File(db, STUDENT_INFORMATION);
    }

    // 从哈希表中删除学生信息
    Remove_Student_From_Memory(db, stuid);

    // 将更新后的内存数据保存回文件
    save_students_to_file(db);
}
```

### remove_student()

有了两个模块我们就可以直接写最终的删除功能了。

```c
// 删除学生信息
void Remove_Student(StudentDB *db, const char *filename, int stuid) {
    // 尝试从内存中删除学生信息
    if (Remove_Student_From_Memory(db, stuid)) {
        // 如果删除成功，保存更新后的数据到文件
        save_students_to_file(db);
        printf("学生 %d 已从内存和文件中删除。\n", stuid);
    } else {
        // 如果内存中未找到学生信息，则从文件加载数据到内存
        Load_From_File(db, filename);
        // 再次尝试从内存中删除
        if (Remove_Student_From_Memory(db, stuid)) {
            // 保存更新后的数据到文件
            save_students_to_file(db);
            printf("学生 %d 已从内存和文件中删除。\n", stuid);
        } else {
            printf("未找到学生 %d，无法删除。\n", stuid);
        }
    }
}
```

## 修改学生信息

思路：老样子，都是从内存和文件两个地方来做修改。

### modify_student_in_memory()

我们通过创建了三个新的实例 new_age，new_name，new_major 来修改哈希表中的需要修改的部分。我们通过判断 student 是否为空，并且学号时候对应来做为修改的条件。

修改完之后提示是否修改成功，然后再将修改之后的内容更新到文件中。

```c
//修改学生信息
void Modify_student_in_memory(StudentDB *db, int stuid, const char *new_name, int new_age, const char *new_major) {
    Student *student = Search_query_in_memory(db, stuid);
    if (student != NULL) {

        // 修改学生信息
        strcpy(student->name, new_name);
        strcpy(student->major, new_major);
        student->age = new_age;

        //提示修改成功
        printf("修改 %d 成功\n", student->stuid);

        // 保存修改后的学生信息到文件
        save_students_to_file(db);
    } else {
        printf("未找到学生 %d！\n", stuid);
    }
}
```



### modify_student_in_file()

这里的代码思路其实和 remove_student_from_file()很像

```c
// 修改学生信息（从文件）
void modify_student_from_file(StudentDB *db, const char *filename, int stuid, const char *new_name, int new_age, const char *new_major) {
    // 如果内存中没有数据，先从文件加载数据到哈希表
    if (db->count == 0) {
        Load_From_File(db, filename);
    }
    // 在内存中修改学生信息
    modify_student_in_memory(db, stuid, new_name, new_age, new_major);

    // 将修改后的学生信息保存回文件
    save_students_to_file(db);
}
```

这里就不过多讲述了，因为太相似了。

### modify_student

这里我们会让修改单个部分，如果你觉得这个字段没有必要去改变的话，可以直接回车。这里主要讲解的地方是输入新信息部分：

我们会先显示当前学号的学生信息，你可以看看这个学生的那个部分是需要修改的，然后通过fgets来防止缓冲区溢出，通过sizeof来确保不会读取超过缓冲区的大小的内容。然后通过`input, "\n")] = '\0'`来判断用户是否直接回车。*`strcspn`*:会找到换行符位置并用`\0`替换，去除输入中的换行符。最后再通过`strncpy`来复制我们字符串。当我们把这个学生的信息都修改好之后，就要可以直接通过save_students_to_file()函数来保存到文件中了。

```c
// 修改学生信息（统一函数）
void Modify_Stu(StudentDB *db, int stuid) {
    Student *stu = Search_query_in_memory(db, stuid);
    if (stu == NULL) {
        // 如果内存中没有，尝试从文件加载
        Load_From_File(db, STUDENT_INFORMATION);
        stu = Search_query_in_memory(db, stuid);
    }
    
    if (stu == NULL) {
        printf("未找到学生 %d\n", stuid);
        return;
    }
    
    // 显示当前信息
    printf("\n当前学生信息:\n");
    printf("学号: %d\n", stu->stuid);
    printf("姓名: %s\n", stu->name);
    printf("年龄: %d\n", stu->age);
    printf("专业: %s\n", stu->major);
    
    // 输入新信息
    char input[100];
    printf("\n输入新信息(直接回车保留原值):\n");
    
    // 姓名
    printf("新姓名 [%s]: ", stu->name);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        input[strcspninput, "\n")] = '\0';
        strncpy(stu->name, input, sizeof(stu->name));
    }
    
    // 年龄
    printf("新年龄 [%d]: ", stu->age);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        stu->age = atoi(input);
    }
    
    // 专业
    printf("新专业 [%s]: ", stu->major);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        input[strcspn(input, "\n")] = '\0';
        strncpy(stu->major, input, sizeof(stu->major));
    }
    
    // 保存到文件
    save_students_to_file(db);
    printf("\n学生信息更新成功!\n");
}
```



## 显示所有学生信息

这个也可以分为两个情景来写，刚写完就查看和从文件中读取再查看

### Show_All_Students_From_Memory()

```c
// 显示内存中的所有学生信息
void Show_All_Stu_From_Memory(StudentDB *db) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Student *current = db->buckets[i];
        while (current != NULL) {
            printf("学号：%d, 姓名：%s, 年龄：%d, 专业：%s\n",
                   current->stuid, current->name, current->age, current->major);
            current = current->next;
        }
    }
}
```

### Show_All_Students_From_Memory()

```c
// 显示文件中的所有学生信息
void Show_All_Stu_From_File(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // 假设文件中的每一行存储一个学生的信息，格式为：学号,姓名,年龄,专业
        printf("%s", line);
    }

    fclose(file);
}
```



## 文件功能

### Load_From_File()

```c
void Load_From_File(StudentDB *db, const char *filename) {
    FILE *file = fopen(filename, "r"); // 以只读模式打开文件
    if (file == NULL) {
        perror("Error opening file"); // 如果文件打开失败，输出错误信息
        return;
    }

    char line[256]; // 定义一个字符数组，用于存储每一行的内容
    while (fgets(line, sizeof(line), file)) { // 使用 fgets 逐行读取文件内容
        Student *newStudent = (Student *)malloc(sizeof(Student)); // 为新的学生信息分配内存
        // 使用 sscanf 解析每一行的数据
        if (sscanf(line, "%d,%[^,],%d,%[^,]", &newStudent->stuid, newStudent->name, &newStudent->age, newStudent->major) == 4) {
            Add_Stu(db, newStudent); // 将解析出来的学生信息添加到内存中的哈希表
        } else {
            free(newStudent); // 如果解析失败，释放为新学生分配的内存
            continue; // 跳过当前循环，继续处理下一行
        }
    }
    fclose(file); // 关闭文件
}
```

### save_to_file()

```c
//将学生信息保存到文件
void save_students_to_file(StudentDB *db) {
    FILE *file = fopen(STUDENT_INFORMATION, "w");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        Student *current = db->buckets[i];
        while (current != NULL) {
            fprintf(file, "%d,%s,%d,%s\n", current->stuid, current->name, current->age, current->major);
            current = current->next;
        }
    }

    fclose(file);
    printf("学生信息已保存到文件！\n");
}
```

## 其他操作

### free_database（）

```C
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
```

这个功能主要是当我们确定要退出程序的时候用来清除内存数值的，但是再这个操作之前，我们一般都会检查一下我们的数据是否已经保存到文件当中了。

## 完整代码：

```c
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

// 添加学生到哈希表
void Add_Stu(StudentDB *db, Student *stu) {
    int hash = stu->stuid % HASH_SIZE;
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
```

