# Zaliczenie Laboratorium II
Masz daną klasę `Resource`, która:
- posiada konstruktor domyślny
- posiada 5 dobrze zdefiniowanych metod specjalnych (choć to akurat nie musi mieć znaczenia dla Twojego rozwiązania)
- posiada metodę o sygnaturze `double get()`
- jest duża, tzn. wartość `sizeof(Resource)` jest większa niż rozmiar stosu

Klasa ta symuluje duży, kosztowny w konstrukcji zasób.
Metoda `get` symuluje wykorzystanie takiego zasobu (np. pobranie wartości zmiennoprzecinkowej z serwera).

Twoim zadaniem jest napisanie klasy `ResourceManager`, która:
- jest właścicielem (czyt. zarządza) obiektu typu `Resource`
- posiada metodę `double get()`, która zwraca wynik zawołania metody `get` obiektu, którym zarządza
- posiada 5 dobrze zdefiniowanych metod specjalnych
- jest mała, tzn. mieści się na stosie

Krótko mówiąc, `ResourceManager` ma być klasą RAII, która zarządza obiektem `Resource`.
Testy weryfikujące poprawność wykonania zadania będą sprawdzały następujące 3 kryteria:
1. Brak wycieków pamięci (1 punkt)
2. Poprawne kopiowanie (2 punkty)
3. Poprawne przenoszenie (2 punkty)

**Uwaga:** Jeżeli kod się nie skompiluje, otrzymujesz 0p.
Jeżeli chcesz wykonać jedynie podpunkty 1 i 2, po prostu nie definiuj konstuktora przenoszącego i przenoszącego operatora przypisania.
Wtedy jako plan B dla przenoszenia zostanie wykonana kopia.
Framework testów to wykryje (sprawdza on liczbę kopii obiektów `Resource`) i nie przyzna punktów za podpunkt 3., ale kod się skompiluje.
Nawiasem mówiąc, jest to jedno (jedyne?) dopuszczale odstępstwo od *rule of 5*.
Gdybyśmy "twardo" wyłączyli semantykę przenoszenia (usunęli wspomniane dwie metody specjalne), to kod się nie skompiluje.
Brak definicji powoduje "miękkie" wyłączenie semantyki przenoszenia, czyli fallback do semantyki kopiowania.
Zaznaczmy, że nie jest to możliwe w drugą stronę (dlaczego?).