#include <iostream>


/*
 Сайты: https://habr.com/ru/articles/490640/
        https://www.geeksforgeeks.org/placement-new-operator-cpp/
 */

/*
 Главное отличие new от malloc:
 - malloc только выделяет память.
 - new выделяет память (operator new) + создает объект в выделенной памяти (constructor). Передаваемый адрес в new может быть ссылкой или указателем.
 - new - присутсвутет строгая гарантия исключений - атомарная операция, либо операция выполнена целиком, либо не выполнена полностью, поэтому нет промежуточного состояние операции. В этом случае идет откат к состоянию, которое было перед выполнением операции, которое вызвало исключение. Например, исключение в конструкторе  - объект не создается, выделенная память освободится и утечки ресурсов не будет.
 C:
 - malloc - allocate memory in heap, return void* - в случае успеха, nullptr - в случае неудачи (закончилась память).
 - free - deallocate memory in heap.
 C++:
 - new - allocate memory in heap + constructor, return T* ptr = new T - в случае успеха, throw bad_alloc - в случае неудачи, конструктор прерывается и вызывается оператор удаления.
 - new[] - allocate memory in heap + constructor для каждого элемента массива. При выбросе исключения в конструкторе, то у всех созданных элементов массива вызывается деструктор в порядке, обратном вызову конструктора, затем выделенная память освобождается.
 - new (std::nothrow) - return T* ptr = new T - в случае успеха, nullptr - в случае неудачи.
 - new (buffer) - allocate memory in placement - не выделяет память, а использует уже выделенную память (static, stack, heap) для вызова конструктора. Могут быть случаи, когда требуется переконструировать объект несколько раз (vector), поэтому в этих случаях размещение нового оператора может быть более эффективным.
 - delete - destructor + deallocate memory in heap.
 - delete[] destructor в порядке, обратном вызову конструктора + deallocate memory in heap.
 
 Перегрузка operator new - отвечает за выделение «сырой памяти». Если в классе перегружается оператор new, то как правило в этом классе также перегружается operator delete. Вызов конструктора объекта остается без изменения.
 Возможные случаи:
 - требуется запрет создания объекта в кучe.
 - память выделяется по особому (нестандартным способом), тогда оператор delete должен также освобождать эту память нестандартным способом.
 Способы перегрузки операторов new/delete:
 - в классах. Наследование new/delete: если операторы определены в базовом классе, а в производном нет, то для производного класса также будет перегружены операторы new/delete, и будут использованы функции для выделения и освобождения памяти, определенные в базовом классе.
 - глобальное (не рекомендуется), их нельзя выносить в namespace.
 
 C++17 TODO:
 void* operator new (std::size_t size, std::align_val_t al);
 void* operator new[](std::size_t size, std::align_val_t al);
 */

struct Data
{
    Data()
    {
        throw std::runtime_error("exception");
    }
    
    int number = 0;
};


namespace CLASS
{
    class A
    {
    private:
        A() = default;
    public:
        explicit A(int number): _number(number){}
        
        static std::unique_ptr<A> Create_Unique()
        {
            // return std::make_unique<A>(); не может вызывать private конструкторы
            return std::unique_ptr<A>(new A());
        }
        
        static std::shared_ptr<A> Create_Shared()
        {
            // return std::make_shared<A>(); не может вызывать private конструкторы
            return std::shared_ptr<A>(new A());
        }
        
        /// Перегруженный оператор new для одиночного объекта
        void* operator new(size_t size) // size – размер выделяемой памяти, кол-во байт выделяемой памяти не обязательно должно совпадать со значением size, т.к. в методе память можно выделять по особому.
        {
            std::cout << "class A " << __func__ << ", size: " << size << std::endl;
            return ::operator new(size);
        }

        /// Перегруженный оператор delete для одиночного объекта
        void operator delete(void* ptr)
        {
            std::cout << __func__ << std::endl;
            ::operator delete(ptr);
        }
        
        /// Перегруженный оператор new для массива объектов
        void *operator new[](const size_t size)
        {
            std::cout << "class A " << __func__ << ", size: " << size << std::endl;
            return ::operator new[](size);
        }
        
        /// Перегруженный оператор delete для массива объектов
        void operator delete[](void* ptr)
        {
            free(ptr);
        }
        
    private:
        int _number = 0;
    };

    class B
    {
    public:
        /// Перегруженный оператор new для одиночного объекта
        void* operator new(size_t size) // size – размер выделяемой памяти, кол-во байт выделяемой памяти не обязательно должно совпадать со значением size, т.к. в методе память можно выделять по особому.
        {
            std::cout << "class B " << __func__ << ", size: " << size << std::endl;
            void *ptr;
            ptr = std::malloc(size);
            
            if (!ptr)
                throw std::bad_alloc();
            
            return ptr;
        }

        /// Перегруженный оператор delete для одиночного объекта
        void operator delete(void* ptr)
        {
            std::cout << __func__ << std::endl;
            free(ptr);
        }
        
        /// Перегруженный оператор new для массива объектов
        void *operator new[](const size_t size)
        {
            std::cout << "Global " << __func__ << ", size: " << size << std::endl;
            void *ptr;
            ptr = ::std::malloc(size);
            
            if (!ptr)
                throw std::bad_alloc();
            
            return ptr;
        }
        
        /// Перегруженный оператор delete для массива объектов
        void operator delete[](void* ptr)
        {
            free(ptr);
        }
    };

    class C
    {
        void* operator new(size_t size) = delete;
        void operator delete(void* ptr) = delete;
        void *operator new[](const size_t size) = delete;
        void operator delete[](void* ptr) = delete;
    };
}

/*
 Операторы new и delete могут быть перегружены глобально, их нельзя выносить в namespace. В этом случае использование глобальных перегруженных операторов new и delete может применяться для любых типов.
 */

/// Перегруженный оператор new для одиночного объекта
void* operator new(size_t size)
{
    std::cout << "Global " << __func__ << ", size: " << size << std::endl;
    void *ptr;
    ptr = std::malloc(size);
    
    if (!ptr)
        throw std::bad_alloc();
    
    return ptr;
}

/// Перегруженный оператор new для одиночного объекта без вывода исключения
void* operator new(std::size_t size, const std::nothrow_t& nth) noexcept
{
    std::cout << "Global " << __func__ << ", size: " << size << std::endl;
    void *ptr;
    ptr = std::malloc(size);
    
    return ptr;
}

/// Перегруженный оператор delete для одиночного объекта
void operator delete(void* ptr) noexcept
{
    std::cout << "Global " << __func__ << std::endl;
    ::std::free(ptr);
}

/// Перегруженный оператор new для массива объектов
void* operator new[](size_t size) // size – размер выделяемой памяти, кол-во байт выделяемой памяти не обязательно должно совпадать со значением size, т.к. в методе память можно выделять по особому.
{
    std::cout << "Global " << __func__ << ", size: " << size << std::endl;
    void *ptr;
    ptr = ::std::malloc(size);
    
    if (!ptr)
        throw std::bad_alloc();
    
    return ptr;
}

/// Перегруженный оператор new для массива объектов  без вывода исключения
void* operator new[](size_t size, const std::nothrow_t& nth) noexcept // size – размер выделяемой памяти, кол-во байт выделяемой памяти не обязательно должно совпадать со значением size, т.к. в методе память можно выделять по особому.
{
    std::cout << "Global " << __func__ << ", size: " << size << std::endl;
    void *ptr;
    ptr = ::std::malloc(size);
    
    return ptr;
}

/// Перегруженный оператор delete для массива объектов
void operator delete[](void* ptr) noexcept
{
    std::cout << "Global " << __func__ << std::endl;
    ::std::free(ptr);
}

void* operator new(std::size_t size, int number, const char* str)
{
    std::cout << "Custom " << __func__ << ", number: " << number << ", str: " << str << std::endl;
    return ::operator new(size);
}
void operator delete(void* p, int number, const char* str)
{
    std::cout << "Custom " << __func__ << ", number: " << number << ", str: " << str << std::endl;
    ::operator delete(p);
}


int main()
{
    /*
     new - присутсвутет строгая гарантия исключений - атомарная операция, либо операция выполнена целиком, либо не выполнена полностью, поэтому нет промежуточного состояние операции. В этом случае идет откат к состоянию, которое было перед выполнением операции, которое вызвало исключение. Например, исключение в конструкторе  - объект не создается, выделенная память освободится и утечки ресурсов не будет.
     */
    {
        Data* data = nullptr;
        try
        {
            data = new Data;
            delete data;
        }
        catch (const std::exception& e)
        {
            std::cout << "Ошибка: " << e.what() << std::endl;
        }
    }
    
    using namespace CLASS;
    
    // make_unique/make_shared
    {
        std::cout << "make_unique/make_shared" << std::endl;
        
        auto a_unique = std::unique_ptr<A>(new A(1)); // new A
        auto a_shared = std::shared_ptr<A>(new A(2)); // new A
        auto a_make_unique = std::make_unique<A>(1); // new global
        auto a_make_shared = std::make_shared<A>(2); // new global
        
        auto b_unique = std::unique_ptr<B[]>(new B[3]); // new B
        auto b_shared = std::shared_ptr<B[]>(new B[3]); // new B
        
        /*
         Запрет создания объекта в кучe
         auto c_unique = std::unique_ptr<B>(new C); // new C
         auto c_shared = std::shared_ptr<B>(new C); // new C
         auto с_mass_unique = std::unique_ptr<C[]>(new C[3]); // new B
         auto c_mass_shared = std::shared_ptr<C[]>(new C[3]); // new B
         */
    }
    // new in class
    {
        std::shared_ptr<A> shared_A(new A(1));
    }
    // custom new/delete
    {
        class Example {/* ... */};
        
        Example* example = new (100, "Example") Example();
        operator delete (example, 100, "Example");
    }
    // new
    {
        std::cout << "new" << std::endl;
        
        float* number = nullptr;
        int* mass = nullptr;
        try
        {
            number = new float(10);
            mass = new int[10];
        }
        catch (std::bad_alloc exception)
        {
            std::cout << exception.what() << std::endl;
        }
        
        delete number;
        delete[] mass;

    }
    // new nothrow
    {
        std::cout << "new nothrow" << std::endl;
        
        float* number = new (std::nothrow) float(10);
        double* mass = new (std::nothrow) double[10];
        
        delete number;
        delete[] mass;
    }
    // new placement - передаваемый адрес в new может быть ссылкой или указателем.
    {
        std::cout << "new placement" << std::endl;
        // number
        {
            [[maybe_unused]] int* number = nullptr;
            [[maybe_unused]] int *null = nullptr;
            
            int* number1 = new int(0);
            int* number2 = nullptr;
            int* number3 = nullptr;
            int* number4 = nullptr;
            
            int number_placement = 0;
            char number_buffer[8];
            
            try
            {
                // number = new(null) int(1); // Error: adress - nullptr
                number1 = new (number1) int(4);
                number2 = new (&number_placement) int(2);
                
                // new (&number_buffer) и new (number_buffer) - для записи числа в начало равносильно
                number3 = new (&number_buffer) int(3);
                number3 = new (number_buffer) int(3); // запишет в массив элемента с индексом 0
                
                // new (&number_buffer + sizeof(int)) и new (number_buffer + sizeof(int)) - для записи в середину НЕ РАВНОСИЛЬНО!
                // number4 = new (&number_buffer + sizeof(int)) int(4);
                number4 = new (number_buffer + sizeof(int)) int(4); // запишет в массив элемента с индексом 4
                
                /*
                 number_buffer: хранениче чисел 100, 200 в 8 байтах
                 100    |200
                 0,1,2,3,4,5,6,7
                 */
            }
            catch (std::bad_alloc exception)
            {
                std::cout << exception.what() << std::endl;
            }
            
            std::cout << "number1: " << *number1 << std::endl;
            std::cout << "number2: " << *number2 << std::endl;
            int* number2_copy = new (&number_placement) int(20); // replacement new memory in number1
            std::cout << "number2: " << *number1 << std::endl;
            std::cout << "number2_copy: " << *number2_copy << std::endl;
            std::cout << "number3: " << *number3 << std::endl;
            std::cout << "number4: " << *number4 << std::endl;
            
            delete number1; // можно удалить переменную, выделенную в куче
            // delete number2; // нельзя удалить переменную, выделенную на стеке
            // delete number3; // нельзя удалить переменную, выделенную на стеке
            // delete number4; // нельзя удалить переменную, выделенную на стеке
        }
        // mass
        {
            constexpr int size = 10;
            int c_mass[size];
            
            int* mass1 = new int[size];
            int* mass2 = nullptr;
            try
            {
                mass1 = new (mass1) int[size]; // запишет один элемент
                
                for (int i = 0; i < size; ++i)
                    mass1[i] = i;
                
                // new (&c_mass) и new (c_mass) - для массива запись равносильно
                mass2 = new (&c_mass) int[size]; // запишет все элементы массива
                mass2 = new (c_mass) int[size]; // запишет все элементы массива
                
                for (int i = 0; i < size; ++i)
                    mass2[i] = i;
            }
            catch (std::bad_alloc exception)
            {
                std::cout << exception.what() << std::endl;
            }
            
            std::cout << "c_mass: " << std::endl;
            for (int i = 0; i < size; ++i)
                std::cout << c_mass[i] << " ";
            std::cout << std::endl;
            
            std::cout << "mass: " << std::endl;
            for (int i = 0; i < size; ++i)
                std::cout << mass1[i] << " ";
            std::cout << std::endl;
            
            delete[] mass1; // можно удалить массив, выделенный в куче
            // delete[] mass2; // нельзя удалить массив, выделенный на стеке
        }
    }

    return 0;
}
