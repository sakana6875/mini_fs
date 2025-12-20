#include "fs.h"
#include "file.h"
#include "dir.h"
#include "path.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

void test_file_operations() {
    // 测试文件创建和打开
    printf("Testing file creation and open...\n");
    int fd = file_open("/testfile.txt", O_CREAT | O_RDWR);
    assert(fd >= 0);  // 文件打开成功

    // 测试文件写入
    const char* write_data = "Hello, file system!";
    size_t write_size = strlen(write_data);
    size_t written = file_write(fd, write_data, write_size);
    assert(written == write_size);  // 成功写入数据

    // 测试文件读
    char read_buf[64] = {0};
    file_lseek(fd, 0, SEEK_SET);  // 将文件指针移到文件开头
    size_t read_size = file_read(fd, read_buf, write_size);
    assert(read_size == write_size);  // 成功读取数据
    assert(strcmp(read_buf, write_data) == 0);  // 检查读出的数据是否与写入的数据一致

    // 测试文件关闭
    assert(file_close(fd) == 0);  // 文件关闭成功
}

void test_file_remove() {
    // 测试文件删除
    printf("Testing file remove...\n");
    int fd = file_open("/testfile.txt", O_CREAT | O_RDWR);
    assert(fd >= 0);  // 文件打开成功

    // 关闭文件
    assert(file_close(fd) == 0);

    // 删除文件
    assert(file_remove("/testfile.txt") == 0);  // 文件删除成功

    // 尝试再次打开已删除的文件，应失败
    fd = file_open("/testfile.txt", O_RDWR);
    assert(fd < 0);  // 文件打开失败
}

void test_file_truncate() {
    // 测试文件裁剪
    printf("Testing file truncate...\n");

    int fd = file_open("/testfile.txt", O_CREAT | O_RDWR);
    assert(fd >= 0);  // 文件打开成功

    const char* write_data = "File truncation test!";
    size_t write_size = strlen(write_data);
    size_t written = file_write(fd, write_data, write_size);
    assert(written == write_size);  // 写入成功

    // 裁剪文件大小
    size_t new_size = 5;
    assert(file_truncate(fd, new_size) == 0);  // 文件裁剪成功

    // 读取裁剪后的文件
    char read_buf[64] = {0};
    file_lseek(fd, 0, SEEK_SET);
    size_t read_size = file_read(fd, read_buf, new_size);
    assert(read_size == new_size);  // 成功读取裁剪后的数据
    assert(strncmp(read_buf, write_data, new_size) == 0);  // 数据一致

    // 关闭文件
    assert(file_close(fd) == 0);  // 文件关闭成功
}

void test_file_append() {
    // 清理
    file_remove("/testfile.txt");

    printf("Testing file append...\n");

    // 第一步：创建文件并写入初始内容
    int fd = file_open("/testfile.txt", O_CREAT | O_RDWR);
    assert(fd >= 0);

    const char* base_data = "File truncation test!";
    size_t base_size = strlen(base_data);
    assert(file_write(fd, base_data, base_size) == base_size);
    assert(file_close(fd) == 0);

    // 第二步：重新打开，追加数据
    fd = file_open("/testfile.txt", O_RDWR); // 不需要 O_CREAT
    assert(fd >= 0);

    const char* append_data = " Appended data.";
    size_t append_size = strlen(append_data);
    size_t appended = file_append(fd, append_data, append_size);
    assert(appended == append_size);

    // 第三步：验证完整内容
    char read_buf[128] = {0};
    file_lseek(fd, 0, SEEK_SET);
    size_t total_size = base_size + append_size; // 21 + 15 = 36
    size_t read_size = file_read(fd, read_buf, total_size);
    assert(read_size == total_size);
    assert(strcmp(read_buf, "File truncation test! Appended data.") == 0);

    assert(file_close(fd) == 0);
}

void test_lookup_path() {
    // 测试路径查找
    printf("Testing path lookup...\n");

    int ino = lookup_path("/testfile.txt");
    assert(ino >= 0);  // 文件存在

    // 测试不存在的文件
    ino = lookup_path("/nonexistent.txt");
    assert(ino < 0);  // 文件不存在
}

int main() {
    // 初始化文件系统
    fs_init();

    // 执行各项测试
    test_file_operations();
    test_file_remove();
    test_file_truncate();
    test_file_append();
    test_lookup_path();

    printf("All tests passed!\n");

    return 0;
}
