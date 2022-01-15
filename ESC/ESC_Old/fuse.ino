void fuseCheck()
{
    if (transCheckState(uTransU) && transCheckState(uTransD))
    {
        fuseTrigger();
    }
    else if (transCheckState(vTransU) && transCheckState(vTransD))
    {
        fuseTrigger();
    }
    else if (transCheckState(wTransU) && transCheckState(wTransD))
    {
        fuseTrigger();
    }
    else
    {
        // digitalWrite(fuseLedPin,!digitalRead(fuseLedPin));
        // Serial.println("Fine...");
        // Serial.println(posState);
    }
    
}

void fuseTrigger()
{
    transTurnOffAll();

    while (1)
    {
        // Serial.println("Fuse Brake!!!");
        digitalWrite(fuseLedPin, HIGH);
        delay(500);
        digitalWrite(fuseLedPin, LOW);
        delay(500);
    }
}