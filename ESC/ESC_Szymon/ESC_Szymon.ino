#include <Wire.h>

//Faza A silnika
int TransistorA_up_pin = 2;
int TransistorA_down_pin = 9;

int TransistorA_up_value = 0;
int TransistorA_down_value = 0;

//Faza B silnika
int TransistorB_up_pin = 3;
int TransistorB_down_pin = 10;

int TransistorB_up_value = 0;
int TransistorB_down_value = 0;

//Faza C silnika
int TransistorC_up_pin = 4;
int TransistorC_down_pin = 11;

int TransistorC_up_value = 0;
int TransistorC_down_value = 0;

//Czujniki halla
int Hall_A = A1;
int Hall_B = A2;
int Hall_C = A3;

//Odczytane PWM
byte PWM_value = 0;
byte max_PWM = 240;
byte min_PWM = 21;

//Wsteczny
bool reverse = true;

//Zabezpieczenia
int Hall_status = 0;
int Hall_status_prev = 0;

unsigned long dead_time = 2;                //Dead time podawany w milisekundach
unsigned long motor_null_time = 1500;       //Sprawdzanie czy silnik zablokował się po określonym czasie w milisekundach
unsigned long hall_null_time = 400;         //Ustalanie limitu czasu gdy wszystkie halle są na zero

unsigned long motor_null_rotation = 0;
unsigned long millis_Hall_status = 0;
unsigned long millis_current = 0;

//Debug czas
// unsigned long millis_prev1 = 0;
// unsigned long millis_prev2 = 0;
// unsigned long millis_prev3 = 0;

void setup()
{
    //Ustawienia Debugowania
    //Serial.begin(115200);

    //Ustawianie tranzystorów ESC jako wyjścia
    pinMode(TransistorA_up_pin, OUTPUT);
    pinMode(TransistorA_down_pin, OUTPUT);
    pinMode(TransistorB_up_pin, OUTPUT);
    pinMode(TransistorB_down_pin, OUTPUT);
    pinMode(TransistorC_up_pin, OUTPUT);
    pinMode(TransistorC_down_pin, OUTPUT);

    digitalWrite(TransistorA_up_pin, LOW);
    analogWrite(TransistorA_down_pin, 0);
    digitalWrite(TransistorB_up_pin, LOW);
    analogWrite(TransistorB_down_pin, 0);
    digitalWrite(TransistorC_up_pin, LOW);
    analogWrite(TransistorC_down_pin, 0);

    //Ustawianie wejść na czujniki halla 
    pinMode(Hall_A, INPUT_PULLUP);
    pinMode(Hall_B, INPUT_PULLUP);
    pinMode(Hall_C, INPUT_PULLUP);

    //Ustawianie: timer1 (pin 9, 10), timer2 (pin 3, 11) na 4kHz
    TCCR1B = TCCR1B & 0b11111000 | 0x02;
    TCCR2B = TCCR2B & 0b11111000 | 0x02;

    //Dioda emergency stop
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    //Interfejs I2C
    Wire.begin(8);
    Wire.onReceive(recive_data);

    //Serial.println("Inicjalizacja ESC!");
}
void loop()
{
    millis_current = millis();

    //Sprawdzanie czy otrzymywana jest wartość PWM większa od zera
    if(PWM_value != 0)
    {
        //Ustalanie kierunku
        if (!reverse)
        {
            esc_set_step(hall_read_status());
        }
        else
        {
            esc_set_step_reverse(hall_read_status());
        }
    }
    else
    {
        motor_null_rotation = millis_current;
        millis_Hall_status = millis_current;
        TransistorA_up_value = 0;
        TransistorA_down_value = 0;
        TransistorB_up_value = 0;
        TransistorB_down_value = 0;
        TransistorC_up_value = 0;
        TransistorC_down_value = 0;
        set_all_transistors_off_delay(0);
    }

    //Wysyłanie wartości do tranzystorów
    send_values_to_pins_with_fuse();
}
void recive_data(int numBytes)
{
    //Otrzymywanie informacji po I2C
    if (Wire.available())
    {
        uint8_t data = Wire.read();

        if (data >= min_PWM && data <= max_PWM)
        {
            PWM_value = data;
        }
        else if (data == 0)
        {
            PWM_value = 0;
        }
        else if (data == 1)
        {
            reverse = true;
        }
        else if (data == 2)
        {
            reverse = false;
        }
        else
        {
            PWM_value = 0;
        }
        //Serial.print("I2C: ");
        //Serial.println(PWM_value);
    }
}
int hall_read_status()
{
    //Odczytywanie wszystkich Halli do zmiennej i zwracanie aktualnej fazy silnika
    /*
    Faza 1 = 4
    Faza 2 = 7
    Faza 3 = 3
    Faza 4 = 5
    Faza 5 = 2
    Faza 6 = 6
    */
    Hall_status = 0;

    Hall_status += (int)digitalRead(Hall_A) * 2;
    Hall_status += (int)digitalRead(Hall_B) * 3;
    Hall_status += (int)digitalRead(Hall_C) * 4;

    //Sprawdza czy zablokował się silnik
    time_fuse(Hall_status);

    switch (Hall_status)
    {
    case 4:
        Hall_status_prev = Hall_status;
        motor_null_rotation = millis_current;
        return 1;
        break;
    case 7:
        Hall_status_prev = Hall_status;
        motor_null_rotation = millis_current;
        return 2;
        break;
    case 3:
        Hall_status_prev = Hall_status;
        motor_null_rotation = millis_current;
        return 3;
        break;
    case 5:
        Hall_status_prev = Hall_status;
        motor_null_rotation = millis_current;
        return 4;
        break;
    case 2:
        Hall_status_prev = Hall_status;
        motor_null_rotation = millis_current;
        return 5;
        break;
    case 6:
        Hall_status_prev = Hall_status;
        motor_null_rotation = millis_current;
        return 6;
        break;
    case 0:
        Hall_status_prev = Hall_status;
        return 10;
        break;
    case 9:
        Hall_status_prev = Hall_status;
        return 11;
        break;
    default:
        emergency_stop();
        break;
    }
}
void esc_set_step(int step)
{
    //Ustawia poszczególne kroki na 3 fazach silnika
    switch (step)
    {
    case 1:
        set_phase_A(0);
        set_phase_B(1);
        set_phase_C(2);
        break;
    case 2:
        set_phase_A(1);
        set_phase_B(0);
        set_phase_C(2);
        break;
    case 3:
        set_phase_A(1);
        set_phase_B(2);
        set_phase_C(0);
        break;
    case 4:
        set_phase_A(0);
        set_phase_B(2);
        set_phase_C(1);
        break;
    case 5:
        set_phase_A(2);
        set_phase_B(0);
        set_phase_C(1);
        break;
    case 6:
        set_phase_A(2);
        set_phase_B(1);
        set_phase_C(0);
        break;
    case 10:
        if (millis_current - motor_null_rotation >= hall_null_time)
        {
            set_phase_A(0);
            set_phase_B(1);
            set_phase_C(2);
        }
        break;
    case 11:
        if (millis_current - motor_null_rotation >= motor_null_time)
        {
            emergency_stop();
        }
        break;
    default:
        emergency_stop();
        break;
    }
}
void esc_set_step_reverse(int step)
{
    //Ustawia poszczególne kroki na 3 fazach silnika
    switch (step)
    {
    case 1:
        set_phase_A(0);
        set_phase_B(2);
        set_phase_C(1);
        break;
    case 2:
        set_phase_A(2);
        set_phase_B(0);
        set_phase_C(1);
        break;
    case 3:
        set_phase_A(2);
        set_phase_B(1);
        set_phase_C(0);
        break;
    case 4:
        set_phase_A(0);
        set_phase_B(1);
        set_phase_C(2);
        break;
    case 5:
        set_phase_A(1);
        set_phase_B(0);
        set_phase_C(2);
        break;
    case 6:
        set_phase_A(1);
        set_phase_B(2);
        set_phase_C(0);
        break;
    case 10:
        if (millis_current - motor_null_rotation >= hall_null_time)
        {
            set_phase_A(0);
            set_phase_B(2);
            set_phase_C(1);
        }
        break;
    case 11:
        if (millis_current - motor_null_rotation >= motor_null_time)
        {
            emergency_stop();
        }
        break;
    default:
        emergency_stop();
        break;
    }
}
int set_phase_A(int status)
{
    //Ustawianie fazy A silnika: status - 1 = GND(masa), status - 2 = VCC(potencjał), status - 0 = NC(niepodłączony)
    //funkcja zwraca ustawiony stan lub włącza awaryjny stop jeśli otrzyma złą wartość
    switch (status)
    {
    case 0:
        TransistorA_up_value = 0;
        TransistorA_down_value = 0;
        return 0;
        break;
    case 1:
        TransistorA_up_value = 0;
        TransistorA_down_value = PWM_value;
        return 1;
        break;
    case 2:
        TransistorA_up_value = 1;
        TransistorA_down_value = 0;
        return 2;
        break;
    default:
        emergency_stop();
        break;
    }
}
int set_phase_B(int status)
{
    //Ustawianie fazy B silnika: status - 1 = GND(masa), status - 2 = VCC(potencjał), status - 0 = NC(niepodłączony)
    //funkcja zwraca ustawiony stan lub włącza awaryjny stop jeśli otrzyma złą wartość
    switch (status)
    {
    case 0:
        TransistorB_up_value = 0;
        TransistorB_down_value = 0;
        return 0;
        break;
    case 1:
        TransistorB_up_value = 0;
        TransistorB_down_value = PWM_value;
        return 1;
        break;
    case 2:
        TransistorB_up_value = 1;
        TransistorB_down_value = 0;
        return 2;
        break;
    default:
        emergency_stop();
        break;
    }
}
int set_phase_C(int status)
{
    //Ustawianie fazy C silnika: status - 1 = GND(masa), status - 2 = VCC(potencjał), status - 0 = NC(niepodłączony)
    //funkcja zwraca ustawiony stan lub włącza awaryjny stop jeśli otrzyma złą wartość
    switch (status)
    {
    case 0:
        TransistorC_up_value = 0;
        TransistorC_down_value = 0;
        return 0;
        break;
    case 1:
        TransistorC_up_value = 0;
        TransistorC_down_value = PWM_value;
        return 1;
        break;
    case 2:
        TransistorC_up_value = 1;
        TransistorC_down_value = 0;
        return 2;
        break;
    default:
        emergency_stop();
        break;
    }
}
void send_values_to_pins_with_fuse()
{
    //Sprawdzanie czy oba tranzystory są wyłączone
    if (TransistorA_up_value == 1 && TransistorA_down_value != 0)
    {
        emergency_stop();
    }
    else if (TransistorB_up_value == 1 && TransistorB_down_value != 0)
    {
        emergency_stop();
    }
    else if (TransistorC_up_value == 1 && TransistorC_down_value != 0)
    {
        emergency_stop();
    }

    //Ustawianie dead time
    if (Hall_status != Hall_status_prev)
    {
        set_all_transistors_off_delay(dead_time);
    }

    //Przekazywanie wartości z zmiennych na piny
    digitalWrite(TransistorA_up_pin, TransistorA_up_value);
    analogWrite(TransistorA_down_pin, TransistorA_down_value);
    digitalWrite(TransistorB_up_pin, TransistorB_up_value);
    analogWrite(TransistorB_down_pin, TransistorB_down_value);
    digitalWrite(TransistorC_up_pin, TransistorC_up_value);
    analogWrite(TransistorC_down_pin, TransistorC_down_value);
}
void time_fuse(int Hall_status)
{
    //Sprawdza czy po określonym czasie zmienił się stan komutacji jeśli nie włączą zabezpieczenie
    if (Hall_status == Hall_status_prev)
    {
        if(millis_current - millis_Hall_status >= motor_null_time)
        {
           emergency_stop();
        }
    }
    else
    {
        millis_Hall_status = millis_current;
    }
}
int set_all_transistors_off_delay(unsigned long dead_time_func)
{
    unsigned long dead_time_counter = 0;

    //Wyłączanie wszystkich tranzystorów na określony czas
    digitalWrite(TransistorA_up_pin, LOW);
    analogWrite(TransistorA_down_pin, 0);
    digitalWrite(TransistorB_up_pin, LOW);
    analogWrite(TransistorB_down_pin, 0);
    digitalWrite(TransistorC_up_pin, LOW);
    analogWrite(TransistorC_down_pin, 0);

    dead_time_counter = millis_current;
    
    //Opóźnienie
    if (dead_time_func != 0)
    {
        while (millis_current - dead_time_counter >= dead_time_func)
        {
            millis_current = millis();
        }
    }
    return 0;
}
void emergency_stop()
{
    //Awaryjne wyłączanie wyszystkiego
    while (true)
    {    
        digitalWrite(TransistorA_up_pin, LOW);
        analogWrite(TransistorA_down_pin, 0);
        digitalWrite(TransistorB_up_pin, LOW);
        analogWrite(TransistorB_down_pin, 0);
        digitalWrite(TransistorC_up_pin, LOW);
        analogWrite(TransistorC_down_pin, 0);

        TransistorA_up_value = 0;
        TransistorA_down_value= 0;
        TransistorB_up_value = 0;
        TransistorB_down_value = 0;
        TransistorC_up_value = 0;
        TransistorC_down_value = 0;

        PWM_value = 0;

        digitalWrite(LED_BUILTIN, HIGH);
    }
}
