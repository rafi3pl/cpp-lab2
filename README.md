# Instrukcja II
## Wstęp
### Zasoby
Na tych zajęciach skupimy się na zarządzaniu zasobami, życiem obiektów oraz metodom do tego służącym. Jest to temat kluczowy w C\+\+, ponieważ język ten nie posiada automatycznego zarządzania zasobami, np. w postaci garbage collectora dostępnego w Javie czy C#. Jest to z jednej strony problematyczne, gdyż zmusza nas do poświęcenia uwagi oraz czasu na sprawy, które mogłyby potencjalnie zostać załatwione przez runtime programu, bez naszego udziału. Z drugiej strony, nie musimy płacić za wygodę takich rozwiązań wydajnością kodu. Jest to zgodne z filozofią C\+\+: *don't pay for what you don't use*.
Zacznijmy od (niekoniecznie ścisłej) definicji: przez zasób rozumiemy byt, który wykorzystujemy do prawidłowego funkcjonowania naszego programu i którego stworzenie i/lub uwolnienie wymaga nietrywialnego nakładu pracy. Przykładami zasobów mogą być:
- **zaalokowana pamięć**
- otwarty plik
- stworzony wątek
- otwarte połączenie sieciowe

Każdy z ww. przykładów zasobów wymaga pracy przy stworzeniu oraz zniszczeniu (np. pamięć trzeba zaalokować i zwolnić). Mamy tu na myśli zarówno pracę programistyczną (napisanie odpowiednich instrukcji) oraz pracę komputera (alokacja pamięci wymaga komunikacji z systemem operacyjnym, który ją przydziela). Drugi typ pracy jest nieunikniony, natomiast ten pierwszy możemy znacznie ograniczyć.

### RAII
Wzorcem projektowym, który nam do tego posłuży jest RAII - *resource acquisition is initialization*. Pomimo dość tajemniczej nazwy, jest to bardzo prosta koncepcja: reprezentujemy zasoby poprzez obiekty, tworzymy zasoby w konstruktorze i zwalniamy w destruktorze. Dzięki temu kod konieczny do zarządzania zasobem piszemy w jednym miejscu, a następnie wykorzystujemy go w trakcie normalnej pracy z danym obiektem. Takie podejście ma następujące zalety:
- Zwiększa poprawność kodu. Obiekty są automatycznie niszczone przy wyjściu ze scope'u, także nigdy nie zapomnimy już np. zwolnić pamięci!
- Zmniejsza liczbę linijek kodu, które musimy napisać.
- Pozwala zachować poprawność programu w sytuacjach wyjątkowych. O wyjątkach mowa będzie dopiero za kilka zajęć, ale na tę chwilę powiedzmy tylko, że są sytuacje, w których jakaś operacja może się nie powieść i program natychmiast przerwie wykonywanie bieżącej funkcji, funkcji, która ją zawołała, itd., aż do momentu, w którym przewidziana jest obsługa takiej wyjątkowej sytuacji. Przed opuszczeniem tych funkcji, program postara się jednak zniszczyć wszystkie obiekty, które zostały dotychczas stworzone. Dzięki zastosowaniu RAII zapewniamy poprawne zwolnienie zasobów, co jest szczególnie ważne, jeżeli program odzyska sprawność. Przykładowo, klient może próbować połączyć się z serwerem przy użyciu funkcji, która alokuje pamięć (np. na adres serwera, przypomnijmy, że `std::string` dynamicznie alokuje swoją zawartość). Serwer nie odpowiada jednak przez jakiś czas, w związku z czym klient decyduje się na porzucenie próby połączenia w trybie awaryjnym. Klient potrafi sobie jednak poradzić z taką sytuacją, np. ponownie wysyłając żądanie do serwera, albo wyświetlając odpowiednią wiadomość użytkownikowi i zajmując się czymś innym. Dzięki odpowiednio napisanemu destruktorowi, można uniknąć wycieku zaalokowanej przez klienta pamięci.

RAII jest jednak sposobem projektowania kodu, nie elementem języka C++ (RAII miało swój początek w C++, ale obecnie wykorzystywane jest także np. w językach Ada i Rust). Zapoznajmy się z konkretną funkcjonalnością oferowaną przez C++, która pozwala stosować RAII, a także inne mechanizmy służące do optymalnego i niekłopotliwego zarządzania zasobami. Skupimy się tu na pamięci, ale innymi zasobami zarządza się analogicznie. Nadzieją autora jest, że po wykonaniu tej instrukcji czytelnik potrafić będzie pisać czysty kod, który nie powoduje wycieków oraz nie wykorzystuje 2kB, tam, gdzie wystarczy 1kB.

## Dynamiczna alokacja pamięci w C++
Zanim przejdziemy do zarządzania zasobami, musimy dowiedzieć się, jak je w ogóle stworzyć. Przypomnijmy, jak wyglądała alokacja pamięci w C:
```C
// Kod w C, w C++ poniższa linijka będzie niepoprawna dla typów innych niż "wbudowane" (int, double, bool, etc.),
// a ściślej mówiąc typów POD (plain old data)
double* liczba = (double*)malloc(sizeof(double));
*liczba = 42.;
// Użyj do czegoś wartości 42
// ...
free(liczba);
```
Powyższe odwołanie do funkcji `malloc` czytamy jako "zaalokuj blok pamięci o wielkości \[rozmiar `double`\] bajtów, a następnie podaj mi adres tego bloku pamięci". Dodatkowo przed przypisanie do zmiennej `liczba` musimy zrzutować otrzymany adres na typ `double*`, gdyż funkcja `malloc` otrzymuje jedynie informację o liczbie bajtów, nie tym, jakiego typu zmienne chcemy tam umieścić (zwraca typ `void*`). Jest to typowe dla języka C - operujemy na gołej pamięci i sami musimy martwić się o typ zmiennych, które w tej pamięci umieszczamy. W C++ kierujemy się fundamentalnie inną filozofią - operujemy na obiektach, których życie zaczyna się od konstruktora, a  kończy na destruktorze. Jeżeli tworzymy krzesło, to chcemy na nim siedzieć, a nie przestawiać jego bity i 4 linijki kodu dalej traktować je jak stół. Zobaczmy, jak dynamiczna alokacja wygląda w C++
```C++
double* liczba = new double{42.};
// Użyj do czegoś wartości 42
// ...
delete liczba;
```
Powyższy kod czytamy jako "zaalokuj dynamicznie pamięć na obiekt typu `double`, stwórz ten obiekt używając konstruktora<sup>1</sup> z argumentem 42, a następnie przypisz adres utworzonego obiektu do zmiennej `liczba`". Dodatkowo możemy zauważyć, że `new` i `delete` są słowami kluczowymi języka, nie bazują na żadnym z nagłówków biblioteki standardowej (np. `stdlib.h`)<sup>2</sup>. Dodajmy też, że składnia
```C++
const unsigned int n      = 100;
double*            wektor = new double[n];
// ...
delete[] wektor;
```
ma tę samą interpretację. Przy alokowaniu tablic musimy jedynie pamiętać o użyciu operatora `delete[]` zamiast `delete`.

Na tych zajęciach spróbujemy napisać klasę `Wektor`, reprezentującą wektor należacy do przestrzeni R<sup>n</sup>, automatycznie dostosowujący swój rozmiar do potrzeb użytkownika (podobnie jak zachowują się wektory np. w matlabie). Zaznaczmy tu, że zachowanie tej klasy będzie inne, niż klasy `std::vector<double>` (z instrukcji nr 5), o czym należy pamiętać.

#### Zadanie 1
Napisz klasę `Wektor`, przechowującą liczby typu `double`, która dynamicznie alokuje pamięć przy konstrukcji i zwalnia ją przy zniszczeniu. Jako argument konstruktora przyjmij zmienną typu całkowitego, reprezentującą ile liczb zmiennoprzecinkowych ma przechowywać wektor (jego długość). Na chwilę obecną dostęp do elementów wektora udostępnij czyniąc publicznym wskaźnik do tablicy, w której przechowujesz elementy wektora.

#### Zadanie 2
Zmodyfikuj konstruktor klasy `Wektor` tak, aby początkowo przypiswał elementom wektora wartość `0.`.

#### Zadanie 3
Dodaj do klasy `Wektor` prywatne pole `dlugosc`, przechowujące informację, ile elementów znajduje się obecnie w wektorze (inicjalizuj to pole argumentem konstruktora). Napisz getter (ale nie setter) dla tego pola.

#### Zadanie 4
Dodaj do klasy `Wektor` publiczną metodę `void print()` która drukuje obecną zawartość wektora (elementy od `0` do `dlugosc - 1`).

#### Zadanie 5
Dodaj do klasy `Wektor` prywatne pole `pojemnosc`, reprezentujące rozmiar tablicy (w sensie liczby `double`'i, nie liczby bajtów) przechowującej elementy wektora (w kolejnych zadaniach `dlugosc` i `pojemnosc` staną się do pewnego stopnia niezależne). Napisz getter (ale nie setter) dla tego pola.

#### Zadanie 6
Dodaj do klasy `Wektor` publiczną metodę `zmienDlugosc`, która przyjmuje zmienną typu całkowitego, reprezentującą nową długość wektora. Jeżeli żądana długość jest mniejsza lub równa obecnej *pojemności* wektora, zmień wartość pola `dlugosc` i wyzeruj "skasowane" w ten sposób elementy (takich elementów może też nie być, np. jeżeli nowa długość jest większa od starej, ale mniejsza niż pojemność). Jeżeli żądana długość jest większa niż obecna *pojemność* wektora, zaalokuj nowy blok pamięci, przepisz do niego istniejące elementy wektora i wyzeruj te nowoutworzone. Nie zapomnij o skasowaniu starego bloku pamięci!

#### Zadanie 7
Przetestuj, czy stworzona przez Ciebie klasa zachowuje się zgodnie z oczekiwaniami. Przydatna do tego może być metoda `print`.

---

<sup>1</sup> Formalnie, wbudowane typy nie mają konstruktorów, ale także możemy je inicjalizować przy użyciu nawiasów `{ }`. Prezentowany kod zadziała analogicznie dla dowolnego innego typu, także nie-POD.

<sup>2</sup> Ściślej mówiąc, operatory `new`, `new[]`, `delete` i `delete[]` są zdefiniowane w nagłówku `new`, ale jest on dołączany nawet bez jawnego zawołania `#include <new>`. Dla ciekawych: operatory te także można przeciążać, *vide* [dokumentacja](https://en.cppreference.com/w/cpp/memory/new/operator_new). Alokacja pamięci jest dość szerokim tematem, w który nie mamy niestety czasu się tu zagłębiać.

## Referencje
Skoro wiemy już jak tworzyć i niszczyć zasoby, zastanówmy się teraz jak korzystać z nich w wydajny sposób. Bardzo często zdarza się, że chcemy wykorzystać jakiś obiekt wewnątrz funkcji (metod tej samej lub innej klasy, bądź też funkcji "wolnostojących"). Rozważmy poniższy kod, znany nam z C:
```C
void print_plus1(int arg)
{
    ++arg;
    printf("%d ", arg);
}

int main()
{
    int a = 0;
    print_plus1(a);
    printf("%d\n", a);
    return 0;
}
```
Taki program wydrukuje oczywiście `1 0`, gdyż funkcja `print_plus1` robi kopię podanego argumentu, także nie zmieni ona wartości zmiennej `a` w funkcji `main`. Spójrzmy na analogiczny kod w C++:
```C++
class T { /* ... */ } ;
void fun(T obj)
{
    // Zrób coś z obj...
}

int main()
{
    T t;
    fun(t);
}
```
Podobnie jak wyżej, funkcja `fun` będzie działać na *kopii* argumentu `t` (o tym, co dokładnie znaczy kopia w kontekście klasy powiemy za chwilę). Jak już ustaliliśmy, obiekt może być "duży", tzn. być właścicielem jakichś zasobów, mieć wiele pól, itd. Wykonywanie jego kopii może nas wtedy kosztować zarówno czas, jak i pamięć. Ponad to, podobnie jak wyżej, nie możemy bezpośrednio modyfikować jego wartości. W języku C rozwiązaniem tego problemu były wskaźniki. Wskaźniki istnieją także w C++, ale dużo bardziej eleganckim rozwiązaniem są referencje. Pozwalają one uniknąć chmary operatorów wzięcia adresu (`&`) oraz dereferencji(`*`). Referencja do obiektu typu `T` ma typ `T&`. Zauważmy, że znak `&` jest tutaj częścią typu, nie operatorem!
Podkreślmy kilka cech referencji:
- Po utworzeniu, korzystamy z referencji dokładnie tak, jak z obiektu, do którego się odnosi.
```C++
int  a     = 0;
int& a_ref = a;

++a_ref;    // Zwiększyliśmy a o 1
a_ref = 42; // Zmieniliśmy wartość a na 42
```
- W przeciwieństwie do wskaźników, referencje nie mogą być puste. Wymusza to na nas przypisanie do referencji obiektu, do którego się odnosi. Poniższy kod się nie skompiluje:
```C++
int a;
int& a_ref; // błąd!
a_ref = a;
```
- Referencja nie może po utworzeniu zacząć odnosić się do innego obiektu. Wynika to z pierwszej wymienionej własności. `a_ref = b` przypisuje wartość `b` do `a`, nie "przypina" referencji do `b` do obiektu `a_ref`. Ponownie, jest to inne zachowanie, niż w przypadku wskaźników.
- Nie istnieje coś takiego jak referencja do referencji (`T&&` oznacza coś zupełnie innego, co zostanie wytłumaczone w dalszej części instrukcji). Znów jest inaczej niż przy wskaźnikach, które można zagnieżdżać dowolnie głęboko (`int****` jest w pełni legalnym typem).

Konsekwencją wypunktowanych powyżej własności jest to, że referencje pojawiają się w praktyce prawie tylko w typach argumentów przyjmowanych oraz zwracanych przez funkcję. W przykładzie powyżej, zmienna `a_ref` jest nam w zasadzie niepotrzebna. Mamy przecież dostęp bezpośrednio do `a`. Wywołanie `fun(a_ref)` **nadal wykona kopię wartości `a`**. Bardzo ważne jest, żeby zrozumieć, dlaczego tak jest. `a_ref` jest typu `int&`, ale `fun` spodziewa się wartości typu `int`. To sygnatura funkcji decyduje o tym, jak zostaną potraktowane argumenty (czy zostaną skopiowane, czy zostanie użyta ich referencja). Aby uniknąć kopii, musimy więc postąpić następująco:
```C++
void print_plus2(int& arg) // Jedyna zmiana w kodzie jest tu!
{
    arg += 2;
    printf("%d ", arg);
}

int main()
{
    int a = 0;
    print_plus2(a);
    printf("%d\n", a);
    return 0;
}
```
Teraz program wydrukuje `2 2`. Zanim wykorzystamy referencje w praktyce, zauważmy jeszcze, że ze względu na cechy uwypuklone powyżej, referencje są nie tylko prostsze, ale też bezpieczniejsze w użyciu niż wskaźniki - trudniej jest stworzyć referencję, która do niczego się nie odnosi. Jest to trudne, lecz nie niemożliwe:
```C++
int& getInt()
{
    int a = 0;
    return a;
}
```
Powyższa funkcja zwraca referencję do lokalnego obiektu, który jest niszczony wraz z końcem jej wykonania. Wartość `int a = getInt()` jest nieokreślona. Szczęśliwie, kompilator powinien wykryć taką sytuację i wydrukować odpowiednie ostrzeżenie.

#### Zadanie 8
Uczyń tablicę, która przechowuje elementy klasy `Wektor` prywatną. Przeciąż odpowiednio operator `[]` tak, aby zwracał referencję do elementu o podanym indeksie. Przyjmij indeksowanie od 0, zauważ jednak, że nic nie stoi na przeszkodzie, aby Twój operator `[]` zaczynał indeksowanie od 1.

#### Zadanie 9
Sprawdź działanie operatora `[]`. Co stanie się, gdy zawołasz `wektor[0] = 42.;`? Co stanie się, gdy zawołasz `double a = wektor[0]; a++;`?

#### Zadanie 10
Zmodyfikuj operator `[]` tak, aby sięgnięcie po element leżący poza obecnym zakresem wektora skutkowało automatycznym zwiększeniem jego długości. Użyj napisanej wcześniej metody `zmienDlugosc`.

#### Zadanie 11
Sprawdź działanie klasy `Wektor`. Zauważ, że nie pozwala ona teraz sięgnąć do niedostępnych miejsc pamięci! W najgorszym wypadku wyczerpiemy dostępną pamięć RAM.

## Listy inicjalizacyjne
Przyjrzyjmy się teraz następującej parze klas:
```C++
struct Kokardka
{
    Kokardka()      { dlugosc = 42; }
    Kokardka(int d) { dlugosc = d; }
    
    int dlugosc;
};

struct Prezent
{
    Prezent(int dk)
    {
        // ***
        k.dlugosc = dk;
    }
    
    Kokardka k;
    // Inne pola ...
};
```
Zadajmy sobie teraz pytanie: przy konstrukcji obiektu typu `Prezent`, jaką długość ma jego kokardka w linijce oznaczonej 3 gwiazdkami? Odpowiedź: 42. Wynika to z faktu, że wszystkie składowe pola klasy `Prezent`, muszą zostać zainicjalizowane przed wykonaniem ciała jego kostruktora. "Pod maską" wołamy zatem domyślny (bezargumentowy) konstruktor klasy `Kokardka`. Gdyby było inaczej, to w konstruktorze prezentu moglibyśmy odnosić się do kokardki, która nie została jeszcze stworzona, co jest logicznie niespójne i skutkowałoby błędami.

#### Zadanie 12
Upewnij się, że kawałek kodu przedstawiony powyżej rzeczywiście działa tak jak twierdzi jego opis (zamień `***` na odpowiednią komendę drukowania). Usuń domyślny konstruktor klasy `Kokardka`. Czy kod się teraz skompiluje?

W zadaniu 12. możemy zauważyć 2 problemy:
- Inicjalizujemy pole `dlugosc` wartością 42, która zaraz jest nadpisywana. Jest to potencjalna niewydajność - gdyby pole to było drogie w konstrukcji (np. gdyby było typu RAII), wykonywalibyśmy drogą operację, która nie byłaby do niczego potrzebna.
- Jeżeli klasa nie posiada domyślnego konstruktora, to, używając poznanych dotychczas elementów języka, nie moglibyśmy użyć jej jako pole innej klasy. Takie zachowanie byłoby niesamowicie problematyczne!

Szczęśliwie, istnieje mechanizm, który pozwala nam sterować konstrukcją składowych pól klasy: lista inicjalizacyjna. Spójrzmy, jak działa:
```C++
struct Kokardka
{
    Kokardka(int d) : dlugosc{d} {}
    
    int dlugosc;
};

struct Prezent
{
    Prezent(int dk) : k{dk} {}
    
    Kokardka k;
    // Inne pola ...
};
```
Jak widać, rozwiązanie to jest nie tylko bardziej wydajne, ale także zwięźlejsze w zapisie.
**Uwaga:** W przypadku inicjalizowania wielu pól klasy w liście inicjalizacyjnej, o kolejności decyduje kolejność deklaracji pól w ciele klasy, nie kolejność występowania w liście inicjalizacyjnej. W związku z tym dobrą praktyką jest inicjalizacja pól jedynie na podstawie argumentów konstruktora, nie innych pól zainicjalizowanych gdzie indziej w liście.

#### Zadanie 13
Zmień konstruktor klasy `Wektor` tak, aby korzystał z listy inicjalizacyjnej.

## Szczególne metody klas
Referencje i wskaźniki pozwalają nam unikać wykonywania kopii obiektów wtedy, gdy nie jest to konieczne. Co jednak zrobić, gdy świadomie chcemy skopiować obiekt? W tej części instrukcji powiemy trochę o 2 szczególnych metodach każdej klasy, które do tego służą. Szczególnych metod jest w sumie 5, jedną z nich - destruktor -już poznaliśmy. Poznamy teraz konstruktor kopiujący i kopiujący operator przypisania. Poniżej zamieszczono kawałek kodu ilustrujący ich definicje.
```C++
class T
{
    // Konstruktor kopiujący
    T(const T&) { /* ... */ }
    
    // Kopiujący operator przypisania
    T& operator=(const T&) { /* ... */ return *this; }
    
    // Destruktor
    ~T() { /* ... */ }
};
```
### Konstruktor kopiujący
Konstruktor kopiujący to konstruktor, który tworzy obiekt na podstawie innego obiektu jego samego typu. Jest on jednoargumentowy - przyjmuje referencję do obiektu, który ma zostać skopiowany. Referencja ta jest stała (`const`), gdyż kopiując obiekt z definicji nie mamy prawa zmienić jego stanu. Zaprezentujmy to na trywialnym przykładzie:
```C++
struct Liczba
{
    int wartosc;
};

int main()
{
    Liczba a{1};
    Liczba b{a};
}
```

### Kopiujący operator przypisania

## Move semantics
### lvalue vs rvalue
### `std::move`
### move constructor, move assignment operator
