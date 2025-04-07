void setup() {
  // put your setup code here, to run once:
  pinMode(D4,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //H
  dot();
  dot();
  dot();
  dot();
  newLetter();
  //E
  dot();
  newLetter();
  //L
  dot();
  dash();
  dot();
  dot();
  newLetter();
  //L
  dot();
  dash();
  dot();
  dot();
  newLetter();
  //O
  dash();
  dash();
  dash();
  newWord();
  //W
  dot();
  dash();
  dash();
  newLetter();
  //O
  dash();
  dash();
  dash();
  newLetter();
  //R
  dot();
  dash();
  dot();
  newLetter();
  //L
  dot();
  dash();
  dot();
  dot();
  newLetter();
  //D
  dash();
  dot();
  dot();
  newLetter();
  //!
  dash();
  dot();
  dash();
  dot();
  dash();
  dash();
  newWord();
}

void dot(){
  digitalWrite(D4,LOW);
  delay(100);
  digitalWrite(D4,HIGH);
  delay(100);
}

void dash(){
  digitalWrite(D4,LOW);
  delay(300);
  digitalWrite(D4,HIGH);
  delay(100);
}

void newLetter(){
  delay(200);
}

void newWord(){
  delay(600);
}
