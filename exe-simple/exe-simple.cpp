// .ini имеет приоритет перед .bat
// если рядом есть .ini файл, то считываются значения ключей в секции [General]: 
// prog - путь до файла (exe, bat, cmd, др.), 
// args - аргументы для файла (спецсимволы типа &^*()/\ должны экранироваться), 
// wdir - рабочая папка файла (если значение в .ini пустое или вовсе не определено, то принимается рабочей папки значения prog), 
// show - видимость окна файла hide, min, max (если значение в .ini пустое или вовсе не определено, то принимается shownormal), 
// если ini файла рядом с .exe не наблюдается, скрыто запускается .bat файл с тем же именем
// имена файлов .bat, .ini должны совпадать с исполяемым
// SimpleIni.h (https://github.com/brofield/simpleini) только для .GetValue
// в конце переменные на всякий случай

#include "SimpleIni.h"
#include <windows.h> // Для ShellExecuteEx и GetModuleFileNameW

// Функция для чтения пути к исполняемому файлу в UNICODE
void GetExecutablePath(std::wstring& path) {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    path.assign(buffer);
}

// Функция для проверки существования файла
bool FileExists(const std::wstring& filePath) {
    DWORD attributes = GetFileAttributesW(filePath.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

int main() {
    // Получаем полный путь к исполняемому файлу (.exe)
    std::wstring exePath;
    GetExecutablePath(exePath);

    // Формируем имена файлов .ini и .bat
    std::wstring iniPath = exePath.substr(0, exePath.find_last_of(L'.')) + L".ini";
    std::wstring batPath = exePath.substr(0, exePath.find_last_of(L'.')) + L".bat";

    // Проверяем наличие файла .ini
    bool hasIni = FileExists(iniPath);

    if (hasIni) { // Если .ini файл найден, работаем по старой схеме
        CSimpleIniW ini(true); // true значит, что будем использовать UTF-16
        ini.SetUnicode();

        // Загрузка файла конфигурации
        ini.LoadFile(iniPath.c_str());

        // Секция "general"
        const wchar_t* section = L"general";

        // Получаем значения из конфига
        std::wstring prog = ini.GetValue(section, L"prog", L"");
        std::wstring args = ini.GetValue(section, L"args", L"");
        std::wstring wdir = ini.GetValue(section, L"wdir", L"");
        std::wstring show = ini.GetValue(section, L"show", L"");

        // Устанавливаем режим отображения окна
        int nShowFlag = SW_SHOWNORMAL;
        if (show == L"hide") {
            nShowFlag = SW_HIDE;
        } else if (show == L"min") {
            nShowFlag = SW_MINIMIZE;
        } else if (show == L"max") {
            nShowFlag = SW_MAXIMIZE;
        }

        // Готовим структуру для запуска программы
        SHELLEXECUTEINFOW sei = {};
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
        sei.lpVerb = L"open";
        sei.lpFile = prog.c_str();
        sei.lpParameters = args.c_str();
        sei.lpDirectory = wdir.c_str();
        sei.nShow = nShowFlag;

        // Запуск программы
        BOOL success = ShellExecuteExW(&sei);
    } else { // Если .ini файла нет, проверяем наличие .bat файла и запускаем его скрытно
        if (FileExists(batPath)) {
            SHELLEXECUTEINFOW sei = {};
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
            sei.lpVerb = L"open";
            sei.lpFile = batPath.c_str();
            sei.nShow = SW_HIDE; // Скрытый запуск батника

            BOOL success = ShellExecuteExW(&sei);
        } else {
            MessageBoxW(nullptr, L"Файл .ini и .bat не найдены.", L"Ошибка", MB_OK | MB_ICONERROR);
            return 1;
        }
    }

    return 0;
}

//https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-showwindow
//Константа 	        Значение	Описание
//SW_HIDE	            0	        Скрывает окно
//SW_SHOWNORMAL	        1	        Отображает нормальное окно
//SW_SHOWMINIMIZED  	2	        Открывает окно минимизированным
//SW_MAXIMIZE	        3	        Максимизирует окно
//SW_SHOWMAXIMIZED  	3	        То же самое, что и SW_MAXIMIZE
//SW_SHOWNOACTIVATE 	4	        Показывает неактивированное окно
//SW_SHOW	            5	        Активирует и отображает окно
//SW_MINIMIZE	        6	        Минимизирует окно
//SW_SHOWMINNOACTIVE	7	        Минимирует окно, не активируя его
//SW_SHOWNA	            8	        Показывает окно в текущем состоянии
//SW_RESTORE	        9	        Восстанавливает прежнее состояние окна
//SW_SHOWDEFAULT	    10	        По умолчанию, выбирает наилучший режим
//SW_FORCEMINIMIZE	    11	        Принудительно минимизирует окно