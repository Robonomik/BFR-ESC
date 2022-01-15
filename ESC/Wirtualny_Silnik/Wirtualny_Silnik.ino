//Wejścia tranzystorów
int Trans_A_up = 2;
int Trans_A_down = 3;
int Trans_B_up = 4;
int Trans_B_down = 5;
int Trans_C_up = 6;
int Trans_C_down = 7;

//Czujniki Halla
int Hall_A = A1;
int Hall_B = A2;
int Hall_C = A3;

//Wartości pinów
String Trans_binary = "000000";
String Hall_binary = "000";

bool hall_start_null = true;

int step = 0;

void setup()
{
  //Inicjalizacja UART
  Serial.begin(115200);

  //Wyjścia halla
  pinMode(Hall_A, OUTPUT);
  pinMode(Hall_B, OUTPUT);
  pinMode(Hall_C, OUTPUT);

  //Wejścia tranzystorów
  pinMode(Trans_A_up, INPUT);
  pinMode(Trans_A_down, INPUT);
  pinMode(Trans_B_up, INPUT);
  pinMode(Trans_B_down, INPUT);
  pinMode(Trans_C_up, INPUT);
  pinMode(Trans_C_down, INPUT);

  Serial.println("Inicjalizacja wirtualnego silnika!");
}
void loop()
{
  //Sprawdzanie czy ma się wykonać sekwencja startowa 
  if (hall_start_null)
  {
    write_hall_pins(0);
    delay(60);
    read_all_pins();

    //Wypisywanie danych
    Serial.println("");
    Serial.println("Komutacja: zerowa");
    Serial.print("Czujniki Halla: ");
    Serial.println(Hall_binary);
    Serial.print("Tranzystory: ");
    Serial.println(Trans_binary);
  }
  else
  {
    write_hall_pins(step);
    delay(60);
    read_all_pins();

    //Wypisywanie danych
    Serial.println("");
    Serial.print("Komutacja: ");
    Serial.println(step);
    Serial.print("Czujniki Halla: ");
    Serial.println(Hall_binary);
    Serial.print("Tranzystory: ");
    Serial.println(Trans_binary);
  }

  int checking = commutation_checking();

  if (hall_start_null && (checking == 1 || checking == 11))
  {
    Serial.println("Wykonano poprawnie sekwencje startową nr: 1");
    hall_start_null = false;
  }

  //Normalna praca
  if(!hall_start_null)
  {
    if (checking > 0 && checking < 7)
    {
      Serial.print("Wykonano poprawnie komutację CW nr: ");
      Serial.print(step);
      Serial.print(", ");
      Serial.println(commutation_checking());
      step++;
    }
    //Sprawdzanie wstecznego
    else if (checking > 10 && checking < 17)
    {
      Serial.print("Wykonano poprawnie komutację CCW nr: ");
      Serial.print(step);
      Serial.print(", ");
      Serial.println(commutation_checking());
      step--;
    }
    else
    {
      Serial.println("Nie wykonano poprawnie ŻADNEJ komutacji!");
    }
  }
  
  if (step > 6)
  {
    step = 1;
  }
  else if (step < 1)
  {
    step = 6;
  }

  delay(140);
}
void write_hall_pins(int motor_step)
{
  //Ustawianie odpowiednich wartości czujników halla
  switch (motor_step)
  {
  case 0:
    digitalWrite(Hall_A, 0);
    digitalWrite(Hall_B, 0);
    digitalWrite(Hall_C, 0);
    break;
  case 1:
    digitalWrite(Hall_A, 0);
    digitalWrite(Hall_B, 0);
    digitalWrite(Hall_C, 1);
    break;
  case 2:
    digitalWrite(Hall_A, 0);
    digitalWrite(Hall_B, 1);
    digitalWrite(Hall_C, 1);
    break;
  case 3:
    digitalWrite(Hall_A, 0);
    digitalWrite(Hall_B, 1);
    digitalWrite(Hall_C, 0);
    break;
  case 4:
    digitalWrite(Hall_A, 1);
    digitalWrite(Hall_B, 1);
    digitalWrite(Hall_C, 0);
    break;
  case 5:
    digitalWrite(Hall_A, 1);
    digitalWrite(Hall_B, 0);
    digitalWrite(Hall_C, 0);
    break;
  case 6:
    digitalWrite(Hall_A, 1);
    digitalWrite(Hall_B, 0);
    digitalWrite(Hall_C, 1);
    break;
  }
}
void read_all_pins()
{
  //Zerowanie
  Trans_binary = "";
  Hall_binary = "";

  //Zczytywanie wejśc tranzystorów
  Trans_binary += digitalRead(Trans_A_up);
  Trans_binary += digitalRead(Trans_A_down);
  Trans_binary += digitalRead(Trans_B_up);
  Trans_binary += digitalRead(Trans_B_down);
  Trans_binary += digitalRead(Trans_C_up);
  Trans_binary += digitalRead(Trans_C_down);

  //Zczytywanie wyjść halla
  Hall_binary += digitalRead(Hall_A);
  Hall_binary += digitalRead(Hall_B);
  Hall_binary += digitalRead(Hall_C);
}
int commutation_checking()
{
  if (Hall_binary == "000" && Trans_binary == "000000")
  {
    return 0;
  }
  else if (Hall_binary == "000" && Trans_binary == "000110")
  {
    return 1;
  }
  else if (Hall_binary == "001" && Trans_binary == "000110")
  {
    return 1;
  }
  else if (Hall_binary == "011" && Trans_binary == "010010")
  {
    return 2;
  }
  else if (Hall_binary == "010" && Trans_binary == "011000")
  {
    return 3;
  }
  else if (Hall_binary == "110" && Trans_binary == "001001")
  {
    return 4;
  }
  else if (Hall_binary == "100" && Trans_binary == "100001")
  {
    return 5;
  }
  else if (Hall_binary == "101" && Trans_binary == "100100")
  {
    return 6;
  }
  //Wsteczny
  // else if (Hall_binary == "000" && Trans_binary == "010010")
  // {
  //   return 11;
  // }
  // else if (Hall_binary == "001" && Trans_binary == "010010")
  // {
  //   return 11;
  // }
  // else if (Hall_binary == "101" && Trans_binary == "000110")
  // {
  //   return 12;
  // }
  // else if (Hall_binary == "100" && Trans_binary == "100100")
  // {
  //   return 13;
  // }
  // else if (Hall_binary == "110" && Trans_binary == "100001")
  // {
  //   return 14;
  // }
  // else if (Hall_binary == "010" && Trans_binary == "001001")
  // {
  //   return 15;
  // }
  // else if (Hall_binary == "011" && Trans_binary == "000110")
  // {
  //   return 16;
  // }
  //Wsteczny alternatywny
  else if (Hall_binary == "000" && Trans_binary == "001001")
  {
    return 11;
  }
  else if (Hall_binary == "001" && Trans_binary == "001001")
  {
    return 11;
  }
  else if (Hall_binary == "011" && Trans_binary == "100001")
  {
    return 12;
  }
  else if (Hall_binary == "010" && Trans_binary == "100100")
  {
    return 13;
  }
  else if (Hall_binary == "110" && Trans_binary == "000110")
  {
    return 14;
  }
  else if (Hall_binary == "100" && Trans_binary == "010010")
  {
    return 15;
  }
  else if (Hall_binary == "101" && Trans_binary == "011000")
  {
    return 16;
  }
}