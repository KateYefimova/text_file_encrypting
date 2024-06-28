
#include <cstdio>  // for fopen, fclose, perror
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <stack>
#include <stdexcept>
#include <windows.h>
#include "Header1.h"
using namespace std;
#define INITIAL_BUFFER_SIZE 100
#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#else
#define CLEAR_COMMAND "clear"
#endif
typedef char* (*encrypt_ptr_t)(char*, int);
typedef char* (*decrypt_ptr_t)(char*, int);

class CaesarCipher {
private:
    HINSTANCE handle;
    encrypt_ptr_t encrypt_ptr;
    decrypt_ptr_t decrypt_ptr;

public:
    bool loadLibrary() {
        handle = LoadLibrary(TEXT("lib4.dll"));
        if (!handle) {
            DWORD error = GetLastError();
            std::cerr << "Error loading DLL. Error code: " << error << std::endl;
            return false;
        }

        // Get function pointers
        encrypt_ptr = (encrypt_ptr_t)GetProcAddress(handle, "encrypt");
        if (!encrypt_ptr) {
            DWORD error = GetLastError();
            std::cerr << "Error loading encrypt function. Error code: " << error << std::endl;
            FreeLibrary(handle);
            handle = nullptr;
            return false;
        }

        decrypt_ptr = (decrypt_ptr_t)GetProcAddress(handle, "decrypt");
        if (!decrypt_ptr) {
            DWORD error = GetLastError();
            std::cerr << "Error loading decrypt function. Error code: " << error << std::endl;
            FreeLibrary(handle);
            handle = nullptr;
            return false;
        }

        return true;
    }
   
    bool processFile(const std::string& inputFilePath, const std::string& outputFilePath, int shift, bool encryptMode) {
        FILE* inputFile = fopen(inputFilePath.c_str(), "r");
        if (!inputFile) {
            std::cerr << "Error opening input file: " << inputFilePath << std::endl;
            return false;
        }

        FILE* outputFile = fopen(outputFilePath.c_str(), "w");
        if (!outputFile) {
            std::cerr << "Error opening output file: " << outputFilePath << std::endl;
            fclose(inputFile);
            return false;
        }

        auto processFunc = encryptMode ? encrypt_ptr : decrypt_ptr;
        char buffer[129];  // Increased buffer size to accommodate null terminator

        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer) - 1, inputFile)) > 0) {
            buffer[bytesRead] = '\0';  // Null-terminate the buffer
            std::cout << "Original buffer: " << buffer << std::endl;

            // Process buffer in place
            char* result = processFunc(buffer, shift);
            

            std::cout << "Processed buffer: " << result << std::endl;
            if (fwrite(result, 1, bytesRead, outputFile) != bytesRead) {
                std::cerr << "Error writing to output file." << std::endl;
                fclose(inputFile);
                fclose(outputFile);
                return false;
            }
            std::cout << "Buffer written to file: " << std::string(result, bytesRead) << std::endl;
        }

        if (ferror(inputFile)) {
            std::cerr << "Error reading from input file." << std::endl;
        }

        fclose(inputFile);
        fclose(outputFile);

        std::cout << "\nFile " << (encryptMode ? "encrypted" : "decrypted") << " successfully." << std::endl;

        return !ferror(inputFile);
    }

    void performOperation() {
        int choice;
        std::cout << "Choose operation:\n1. Encrypt\n2. Decrypt\nEnter choice: ";
        std::cin >> choice;

        bool encryptMode = (choice == 1);

        std::string inputFilePath, outputFilePath;
        int shift;

        std::cout << "Enter input file path: ";
        std::cin >> inputFilePath;

        std::cout << "Enter output file path: ";
        std::cin >> outputFilePath;

        std::cout << "Enter shift key: ";
        std::cin >> shift;

        if (processFile(inputFilePath, outputFilePath, shift, encryptMode)) {
            std::cout << "Operation completed successfully." << std::endl;
        }
        else {
            std::cerr << "Operation failed." << std::endl;
        }
    }

    ~CaesarCipher() {
        if (handle) {
            FreeLibrary(handle);
        }
    }
};
class FileManager {
public:
    static void save_to_file(const char* filename, char** lines, int current_line) {
        FILE* file = fopen(filename, "w");
        if (!file) {
            throw std::runtime_error("Error opening file for writing");
        }

        for (int i = 0; i < current_line; ++i) {
            if (lines[i] != nullptr) {
                fprintf(file, "%s\n", lines[i]);
            }
        }

        fclose(file);
        std::cout << "Text has been saved successfully to " << filename << std::endl;
    }

    static char** load_from_file(const char* filename, int& current_line) {
        FILE* file = fopen(filename, "r");
        if (!file) {
            throw std::runtime_error("Error opening file for reading");
        }

        char** lines = nullptr;
        current_line = 0;

        size_t buffer_size = INITIAL_BUFFER_SIZE;
        char* buffer = (char*)malloc(buffer_size);
        if (!buffer) {
            fclose(file);
            throw std::runtime_error("Memory allocation failed");
        }

        lines = (char**)malloc(sizeof(char*));
        if (!lines) {
            fclose(file);
            free(buffer);
            throw std::runtime_error("Memory allocation failed");
        }

        while (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            while (len > 0 && buffer[len - 1] != '\n') {
                buffer_size *= 2;
                buffer = (char*)realloc(buffer, buffer_size);
                if (!buffer) {
                    fclose(file);
                    free(buffer);
                    for (int i = 0; i < current_line; ++i) {
                        free(lines[i]);
                    }
                    free(lines);
                    throw std::runtime_error("Memory allocation failed");
                }
                if (!fgets(buffer + len, buffer_size - len, file)) {
                    break;
                }
                len = strlen(buffer);
            }

            buffer[strcspn(buffer, "\n")] = '\0';

            lines[current_line] = strdup(buffer);
            if (!lines[current_line]) {
                fclose(file);
                free(buffer);
                for (int i = 0; i < current_line; ++i) {
                    free(lines[i]);
                }
                free(lines);
                throw std::runtime_error("Memory allocation failed");
            }
            ++current_line;

            // Resize the lines array if needed
            char** temp = (char**)realloc(lines, (current_line + 1) * sizeof(char*));
            if (!temp) {
                fclose(file);
                free(buffer);
                for (int i = 0; i < current_line; ++i) {
                    free(lines[i]);
                }
                free(lines);
                throw std::runtime_error("Memory allocation failed");
            }
            lines = temp;
        }

        free(buffer);
        fclose(file);

        return lines;
    }
};

class TextEditor {
private:
    int current_line;
    char* clipboard;
    char** lines;
    int cursor_line;
    int cursor_index;
    std::stack<char**> undo_stack;
    std::stack<char**> redo_stack;
    std::stack<int> undo_line_counts;
    std::stack<int> redo_line_counts;

    void save_snapshot(std::stack<char**>& stack, std::stack<int>& line_count_stack) {
        char** snapshot = new char* [current_line];
        for (int i = 0; i < current_line; ++i) {
            size_t length = strlen(lines[i]) + 1;
            snapshot[i] = new char[length];
            strcpy_s(snapshot[i], length, lines[i]);
        }
        stack.push(snapshot);
        line_count_stack.push(current_line);
    }

    void restore_snapshot(std::stack<char**>& stack, std::stack<int>& line_count_stack) {
        if (stack.empty()) {
            return;
        }

        if (lines != nullptr) {
            for (int i = 0; i < current_line; ++i) {
                delete[] lines[i];
            }
            delete[] lines;
        }

        lines = stack.top();
        current_line = line_count_stack.top();
        stack.pop();
        line_count_stack.pop();
    }

    void clear_stack(std::stack<char**>& stack, std::stack<int>& line_count_stack) {
        while (!stack.empty()) {
            char** snapshot = stack.top();
            int count = line_count_stack.top();
            for (int i = 0; i < count; ++i) {
                delete[] snapshot[i];
            }
            delete[] snapshot;
            stack.pop();
            line_count_stack.pop();
        }
    }
public:
    TextEditor() {
        current_line = 0;
        clipboard = nullptr;
        lines = nullptr;
        cursor_line = 0;
        cursor_index = 0;
    }

    ~TextEditor() {
        if (lines != nullptr) {
            for (int i = 0; i < current_line; i++) {
                delete[] lines[i];
            }
            delete[] lines;
        }
        if (clipboard != nullptr) {
            delete[] clipboard;
        }
        clear_stack(undo_stack, undo_line_counts);
        clear_stack(redo_stack, redo_line_counts);
    }


    void save_to_file(const char* filename) {
        try {
            FileManager::save_to_file(filename, lines, current_line);
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void load_from_file(const char* filename) {
        try {
            int new_current_line;
            char** new_lines = FileManager::load_from_file(filename, new_current_line);

            // Clean up existing lines
            if (lines != nullptr) {
                for (int i = 0; i < current_line; i++) {
                    delete[] lines[i];
                }
                delete[] lines;
            }

            lines = new_lines;
            current_line = new_current_line;

            std::cout << "Text loaded successfully from " << filename << ":" << std::endl;
            for (int i = 0; i < current_line; i++) {
                std::cout << lines[i] << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void clear_console() {
        system(CLEAR_COMMAND);
    }

    void append_text(const char* to_append) {
        if (current_line == 0) {
            std::cout << "No lines to append text to." << std::endl;
            return;
        }

        size_t original_length = strlen(lines[current_line - 1]);
        size_t to_append_length = strlen(to_append);
        char* temp = (char*)realloc(lines[current_line - 1], original_length + to_append_length + 2);
        if (temp == nullptr) {
            std::cout << "Memory allocation failed" << std::endl;
            return;
        }

        lines[current_line - 1] = temp;

        if (original_length > 0) {
            strcat_s(lines[current_line - 1], original_length + to_append_length + 2, " ");
        }
        strcat_s(lines[current_line - 1], original_length + to_append_length + 2, to_append);
    }

    void start_new_line() {
        save_snapshot(undo_stack, undo_line_counts);
        clear_stack(redo_stack, redo_line_counts);
        char** temp = new char* [current_line + 1];
        for (int i = 0; i < current_line; i++) {
            temp[i] = lines[i];
        }
        temp[current_line] = new char[INITIAL_BUFFER_SIZE];
        strcpy_s(temp[current_line], INITIAL_BUFFER_SIZE, "");
        delete[] lines;
        lines = temp;
        current_line++;
    }



    void insert_text(int line, int index, const char* text) {
        save_snapshot(undo_stack, undo_line_counts);
        clear_stack(redo_stack, redo_line_counts);
        if (line >= current_line || line < 0) {
            std::cout << "Invalid line number." << std::endl;
            return;
        }
        size_t line_length = strlen(lines[line]);
        size_t text_length = strlen(text);

        if (index > line_length || index < 0) {
            std::cout << "Invalid index." << std::endl;
            return;
        }

        lines[line] = (char*)realloc(lines[line], line_length + text_length + 1);
        if (lines[line] == nullptr) {
            std::cout << "Memory allocation failed" << std::endl;
            return;
        }

        memmove(&lines[line][index + text_length], &lines[line][index], line_length - index + 1);
        memcpy(&lines[line][index], text, text_length);
    }

    void insert_text_with_replacement(int line, int index, const char* text) {
        save_snapshot(undo_stack, undo_line_counts);
        clear_stack(redo_stack, redo_line_counts);
        if (line >= current_line || line < 0) {
            std::cout << "Invalid line number." << std::endl;
            return;
        }
        size_t line_length = strlen(lines[line]);
        size_t text_length = strlen(text);

        if (index > line_length || index < 0) {
            std::cout << "Invalid index." << std::endl;
            return;
        }

        if (index + text_length > line_length) {
            lines[line] = (char*)realloc(lines[line], index + text_length + 1);
            if (lines[line] == nullptr) {
                std::cout << "Memory allocation failed" << std::endl;
                return;
            }
        }
        memcpy(&lines[line][index], text, text_length);
        if (index + text_length > line_length) {
            lines[line][index + text_length] = '\0';
        }
    }

    void search_text(const char* text_to_search) {
        int found = 0;
        size_t search_len = strlen(text_to_search);
        for (int i = 0; i < current_line; i++) {
            char* pos = lines[i];
            while ((pos = strstr(pos, text_to_search)) != nullptr) {
                int index = pos - lines[i];
                std::cout << "Line " << i << ", Index " << index << std::endl;
                pos += search_len;
                found = 1;
            }
        }
        if (!found) {
            std::cout << "Text not found." << std::endl;
        }
    }

    void delete_text(int line, int index, int length) {
        save_snapshot(undo_stack, undo_line_counts);
        clear_stack(redo_stack, redo_line_counts);
        if (line >= current_line || line < 0) {
            std::cout << "Invalid line number." << std::endl;
            return;
        }
        size_t line_length = strlen(lines[line]);
        if (index >= line_length || index < 0 || length < 0 || index + length > line_length) {
            std::cout << "Invalid index or length." << std::endl;
            return;
        }

        memmove(&lines[line][index], &lines[line][index + length], line_length - index - length + 1);
        lines[line] = (char*)realloc(lines[line], line_length - length + 1);
        if (lines[line] == nullptr) {
            std::cout << "Memory allocation failed" << std::endl;
            return;
        }
    }

    void copy_text(int line, int index, int length) {
        if (line >= current_line || line < 0) {
            std::cout << "Invalid line number." << std::endl;
            return;
        }
        size_t line_length = strlen(lines[line]);
        if (index >= line_length || index < 0 || length < 0 || index + length > line_length) {
            std::cout << "Invalid index or length." << std::endl;
            return;
        }

        if (clipboard != nullptr) {
            delete[] clipboard;
        }
        clipboard = new char[length + 1];
        if (clipboard == nullptr) {
            std::cout << "Memory allocation failed" << std::endl;
            return;
        }

        strncpy_s(clipboard, length + 1, &lines[line][index], length);
        clipboard[length] = '\0';
    }

    void paste_text(int line, int index) {
        if (clipboard == nullptr) {
            std::cout << "Clipboard is empty." << std::endl;
            return;
        }
        insert_text(line, index, clipboard);
    }

    void cut_text(int line, int index, int length) {
        copy_text(line, index, length);
        delete_text(line, index, length);
    }

    void undo() {
        if (undo_stack.empty()) {
            std::cout << "No actions to undo." << std::endl;
            return;
        }
        save_snapshot(redo_stack, redo_line_counts);
        restore_snapshot(undo_stack, undo_line_counts);
    }

    void redo() {
        if (redo_stack.empty()) {
            std::cout << "No actions to redo." << std::endl;
            return;
        }
        save_snapshot(undo_stack, undo_line_counts);
        restore_snapshot(redo_stack, redo_line_counts);
    }
    void show_menu() {
        std::cout << "Choose the command:" << std::endl;
        std::cout << "1. Append text symbols to the end" << std::endl;
        std::cout << "2. Start new line" << std::endl;
        std::cout << "3. Use files to save the information" << std::endl;
        std::cout << "4. Use files to load the information" << std::endl;
        std::cout << "5. Print the current text to console" << std::endl;
        std::cout << "6. Insert the text by line and symbol index" << std::endl;
        std::cout << "7. Search" << std::endl;
        std::cout << "8. Delete text by line, index, and length" << std::endl; // New command
        std::cout << "9. Clear console" << std::endl;
        std::cout << "10. Undo" << std::endl;
        std::cout << "11. Redo" << std::endl;
        std::cout << "12. Cut text" << std::endl;
        std::cout << "13. Paste text" << std::endl;
        std::cout << "14. Copy text" << std::endl;
        std::cout << "15. Insert with replacement" << std::endl;
        std::cout << "16. Show menu" << std::endl;
        std::cout << "17. Encrypt/decrypt" << std::endl;
        std::cout << "18. Exit" << std::endl;
    }
    int set_cursor() {
        
        std::pair<int, int> cursor_position = get_cursor_position();
        int line = cursor_position.first;
        int index = cursor_position.second;
        return line, index;
    }

    char* read_line() {
        size_t size = INITIAL_BUFFER_SIZE;
        size_t len = 0;
        char* buffer = (char*)malloc(size);
        if (!buffer) {
            std::cout << "Memory allocation failed" << std::endl;
            exit(EXIT_FAILURE);
        }

        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            buffer[len++] = ch;
            if (len == size) {
                size *= 2;
                char* temp_buffer = (char*)realloc(buffer, size);
                if (!temp_buffer) {
                    std::cout << "Memory reallocation failed" << std::endl;
                    free(buffer);
                    exit(EXIT_FAILURE);
                }
                buffer = temp_buffer;
            }
        }
        buffer[len] = '\0';
        return buffer;
    }
    void display_text() const {
        std::cout << "Current text:" << std::endl;
        bool hasLines = false;

        for (int i = 0; i < current_line; i++) {
            if (lines[i] != nullptr) {
                std::cout << lines[i] << std::endl;
                hasLines = true;
            }
        }

        if (!hasLines) {
            std::cout << "No lines" << std::endl;
        }
    }
    std::pair<int, int> get_cursor_position() {
        return { cursor_line, cursor_index };
    }

    void run();

};

int main() {
    TextEditor editor;
    editor.run();

    return 0;
}

inline void TextEditor::run() {
    int command;
    while (true) {
        show_menu();
        std::cout << "Enter the command: ";

        std::cin >> command;
        if (command < 1 || command > 17) {
            std::cout << "Invalid command. Please enter a number between 1 and 17." << std::endl;
            continue;
        }
        show_menu();
        switch (command) {
        case 1: {
            clear_console();
            std::cout << "Enter text to append: ";
            std::cin.ignore();

            if (current_line == 0) {
                start_new_line();
            }

            char* to_append = read_line();
            append_text(to_append);
            free(to_append);
            break;
        }
        case 2: {
            clear_console();
            start_new_line();
            std::cout << "New line started" << std::endl;
            break;
        }
        case 3: {
            clear_console();
            std::cout << "Enter the file name for saving: ";
            std::cin.ignore();
            char* filename = read_line();

            save_to_file(filename);
            free(filename);
            break;
        }
        case 4: {
            clear_console();
            std::cout << "Enter the file name for loading: ";
            std::cin.ignore();
            char* load_filename = read_line();

            load_from_file(load_filename);
            free(load_filename);
            break;
        }
        case 5: {
            clear_console();
            display_text();
            break;
        }
        case 6: {

            std::pair<int, int> cursor_position = get_cursor_position();
            int line = cursor_position.first;
            int index = cursor_position.second;
            std::cout << "Enter text to insert:" << std::endl;
            std::cin.ignore();
            char* text = read_line();
            insert_text(line, index, text);
            free(text);
            break;
        }
        case 7: {
            clear_console();
            std::cout << "Enter text to search:" << std::endl;
            char* text_to_search = read_line();
            std::cout << "Text found at these positions:" << std::endl;
            search_text(text_to_search);
            free(text_to_search);
            break;
        }
        case 8: {
            
            std::pair<int, int> cursor_position = get_cursor_position();
            int line = cursor_position.first;
            int index = cursor_position.second;
            std::cout << "Choose length to delete:" << std::endl;
            std::cin.ignore();
            int  length;
            if (scanf_s("%d", &length) != 1) {
                std::cout << "Invalid input. Please enter one numbers." << std::endl;
                while (getchar() != '\n');
                break;
            }
            getchar();
            delete_text(line, index, length);
            break;
        }
        case 9: {
            clear_console();
            show_menu();
            break;
        }
        case 10: // Undo
            clear_console();
            undo();
            break;
        case 11: // Redo
            clear_console();
            redo();
            break;
        case 12: {
            
            std::pair<int, int> cursor_position = get_cursor_position();
            int line = cursor_position.first;
            int index = cursor_position.second;
            std::cout << "Choose  length to cut:" << std::endl;
            std::cin.ignore();
            int  length;
            if (scanf_s("%d", &length) != 1) {
                std::cout << "Invalid input. Please enter one numbers." << std::endl;
                while (getchar() != '\n');
                break;
            }
            getchar();
            cut_text(line, index, length);
            break;
        }
        case 13: {
           
            std::pair<int, int> cursor_position = get_cursor_position();
            int line = cursor_position.first;
            int index = cursor_position.second;
            paste_text(line, index);
            break;
        }
        case 14: {
           
            std::pair<int, int> cursor_position = get_cursor_position();
            int line = cursor_position.first;
            int index = cursor_position.second;
            std::cout << "Choose length to copy:" << std::endl;
            std::cin.ignore();
            int  length;
            if (scanf_s("%d", &length) != 1) {
                std::cout << "Invalid input. Please enter one numbers." << std::endl;
                while (getchar() != '\n');
                break;
            }
            getchar();
            copy_text(line, index, length);
            break;
        }
        case 15: {
           
            std::pair<int, int> cursor_position = get_cursor_position();
            int line = cursor_position.first;
            int index = cursor_position.second;

            std::cout << "Enter text to insert with replacement:" << std::endl;
            std::cin.ignore();
            char* text = read_line();
            insert_text_with_replacement(line, index, text);
            free(text);
            break;
        }
        case 16: {
            show_menu();
            break;
        }
        case 18:
            std::cout << "Exiting..." << std::endl;
            exit(0);
        case 17: {
            CaesarCipher cipher;
            if (!cipher.loadLibrary()) {
                std::cerr << "Failed to load the DLL." << std::endl;
                break;
            }
            cipher.performOperation();
            break;
        }
        default:
            std::cout << "The command is not implemented." << std::endl;
        }
    }
}