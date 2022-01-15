int hall_1_pin = A1;
int hall_2_pin = A2;
int hall_3_pin = A3;

int phase_A_up_pin = 2;
int phase_A_down_pin = 3;
int phase_B_up_pin = 4;
int phase_B_down_pin = 5;
int phase_C_up_pin = 6;
int phase_C_down_pin = 7;

String hall_binary = "000";
String phase_binary = "000000";

int step = 4;
bool motor_null = true;

void setup()
{
  Serial.begin(115200);
  pinMode(hall_1_pin, OUTPUT);
  pinMode(hall_2_pin, OUTPUT);
  pinMode(hall_3_pin, OUTPUT);
  pinMode(phase_A_up_pin, INPUT);
  pinMode(phase_A_down_pin, INPUT);
  pinMode(phase_B_up_pin, INPUT);
  pinMode(phase_B_down_pin, INPUT);
  pinMode(phase_C_up_pin, INPUT);
  pinMode(phase_C_down_pin, INPUT);
  Serial.print("First Step: ");
  Serial.println(step);
}

void loop()
{
  step++;
  
  if(motor_null)
  {
    digitalWrite(hall_1_pin, 0);
    digitalWrite(hall_2_pin, 0);
    digitalWrite(hall_3_pin, 0);
    if (phase_binary != "000000")
    {
      Serial.print("Start motor: ");
      Serial.println(phase_binary);
      motor_null = false;
    }
  }
  else
  {
    set_pins_hall();
  }
  delay(100);
  read_pins_binary();
  validating_phase();
}

void read_pins_binary()
{

  int hall_1 = digitalRead(hall_1_pin);
  int hall_2 = digitalRead(hall_2_pin);
  int hall_3 = digitalRead(hall_3_pin);

  int A_up = digitalRead(phase_A_up_pin);
  int A_down = digitalRead(phase_A_down_pin);
  int B_up = digitalRead(phase_B_up_pin);
  int B_down = digitalRead(phase_B_down_pin);
  int C_up = digitalRead(phase_C_up_pin);
  int C_down = digitalRead(phase_C_down_pin);

  hall_binary = ""; 
  hall_binary += hall_1;
  hall_binary += hall_2;
  hall_binary += hall_3;

  Serial.print("Czujniki Halla: ");
  Serial.println(hall_binary);

  phase_binary = "";
  phase_binary += String(A_up);
  phase_binary += String(A_down);
  phase_binary += String(B_up);
  phase_binary += String(B_down);
  phase_binary += String(C_up);
  phase_binary += String(C_down);

  Serial.print("Motor: ");
  Serial.println(phase_binary);
}

void set_pins_hall()
{
  switch (step)
  {
    case 1:
      digitalWrite(hall_1_pin, 1);
      digitalWrite(hall_2_pin, 0);
      digitalWrite(hall_3_pin, 1);
      break;
    case 2:
      digitalWrite(hall_1_pin, 1);
      digitalWrite(hall_2_pin, 0);
      digitalWrite(hall_3_pin, 0);
      break;
    case 3:
      digitalWrite(hall_1_pin, 1);
      digitalWrite(hall_2_pin, 1);
      digitalWrite(hall_3_pin, 0);
      break;
    case 4:
      digitalWrite(hall_1_pin, 0);
      digitalWrite(hall_2_pin, 1);
      digitalWrite(hall_3_pin, 0);
      break;
    case 5:
      digitalWrite(hall_1_pin, 0);
      digitalWrite(hall_2_pin, 1);
      digitalWrite(hall_3_pin, 1);
      break;
    case 6:
      digitalWrite(hall_1_pin, 0);
      digitalWrite(hall_2_pin, 0);
      digitalWrite(hall_3_pin, 1);
      break;
  }
}

void validating_phase()
{
  switch (step)
  {
    case 1:
      if (hall_binary == "101" && phase_binary == "000110") 
      {
        Serial.println("Faza 1 jest poprawna!");
        Serial.println("");
      }
      else
      {
        Serial.println("Faza 1 jest NIE poprawna!");
        Serial.println("");
        step = 0;
      }
      break;
    case 2:
      if (hall_binary == "100" && phase_binary == "100100") 
      {
        Serial.println("Faza 2 jest poprawna!");
        Serial.println("");
      }
      else
      {
        Serial.println("Faza 2 jest NIE poprawna!");
        Serial.println("");
        step = 1;
      }
      break;
    case 3:
      if (hall_binary == "110" && phase_binary == "100001") 
      {
        Serial.println("Faza 3 jest poprawna!");
        Serial.println("");
      }
      else
      {
        Serial.println("Faza 3 jest NIE poprawna!");
        Serial.println("");
        step = 2;
      }
      break;
    case 4:
      if (hall_binary == "010" && phase_binary == "001001") 
      {
        Serial.println("Faza 4 jest poprawna!");
        Serial.println("");
      }
      else
      {
        Serial.println("Faza 4 jest NIE poprawna!");
        Serial.println("");
        step = 3;
      }
      break;
    case 5:
      if (hall_binary == "011" && phase_binary == "011000") 
      {
        Serial.println("Faza 5 jest poprawna!");
        Serial.println("");
      }
      else
      {
        Serial.println("Faza 5 jest NIE poprawna!");
        Serial.println("");
        step = 4;
      }
      break;
    case 6:
      if (hall_binary == "001" && phase_binary == "010010") 
      {
        Serial.println("Faza 6 jest poprawna!");
        Serial.println("");
        step = 0;
      }
      else
      {
        Serial.println("Faza 6 jest NIE poprawna!");
        Serial.println("");
        step = 5;
      }
      break;
    default:
      Serial.println("");
      Serial.print("Nie prawidłowa faza numer: ");
      Serial.println(step);
      step = 0;
      break;
  }
}
