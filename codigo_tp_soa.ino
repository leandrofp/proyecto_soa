//LIMITE DISTANCIA
#define LIMITE                         10    // Limite en cm para que el sensor de ultrasonido coinsidere que hay un vehiculo
//LIMITE LUZ
#define LIMITE_LUZ                     600   // Limite para que la resistencia LDR coinsidere que hay poca luz

//TIEMPOS DE CRUCE
#define TIEMPO_ESPERA                  3000  // Tiempo que espera un auto para cruzar la avenida
#define TIEMPO_PASO_CA_CO              6000  // Tiempo que permanece en verde el semaforo de la calle para que los autos crucen la avenida cuando hay pocos autos
#define TIEMPO_PASO_CA_LA              8000  // Tiempo que permanece en verde el semaforo de la calle para que los autos crucen la avenida cuando hay muchos autos
#define TIEMPO_PASO_AV                 10000 // Tiempo que permanece en verde el semaforo de la avenida para que los autos crucen la calle
#define TIEMPO_PASO_PEATON             1000  // Tiempo de cruce que se le otorga al peaton cuando presiona el boton
#define TIEMPO_EMERGENCIA              15000 // Tiempo de paso que se asigna cuando se cambia el estado desde la app

 
//NO CAMBIAR
#define SONIDO_RAP                     100   // Se puede cruzar
#define SONIDO_LENTO                   1000  // No se puede cruzar

//SENSORES
#define emisor_s1       8
#define receptor_s1     9

#define emisor_s2       10
#define receptor_s2     11

//SEMAFOROS
#define Rav_Vca    3  //ROJO AVENIDA Y VERDE CALLE
#define ama        4 // AMARILLOS
#define Vav_Rca    5 // VERDE AVENIDA Y ROJO CALLE

//Bocinas
#define bocina_CA  6
#define bocina_AV  7

//Alumbrado
#define luz 12
#define sensor_ldr A5


//DECLARO LAS FUNCIONES
long TomarDatosSensor();
void MostrarDatos(long, int);
void AbrirCalle(int);
void AbrirAvenida();
void AbrirAvenida(int);
boolean HayAuto(long);
boolean ExcedioEspera(unsigned long, unsigned long);
void CambiarSemaforo(const char);
void EmitirSonidoLento(int);
void EmitirSonidoRap(int);
void BocinasLow();
void atender();
void LeerBuffer();
void Actuar();
void VerificarEmergencia();
void VerificarAlumbrado();

unsigned long t_rap = 0, t_lento = 0;
volatile int hay_peaton = 0;
volatile char datoBT;
volatile int valor_ldr;

void setup() {
       Serial.begin (9600);
       
       //SETEO LOS SENSORES
       //SENSOR 1
       pinMode(emisor_s1, OUTPUT);
       pinMode(receptor_s1, INPUT);

       //SENSOR 2
       pinMode(emisor_s2, OUTPUT);
       pinMode(receptor_s2, INPUT);
       
       //SETEO LOS PIN COMO SALIDAS DE LUCES
       pinMode(Rav_Vca, OUTPUT);
       pinMode(ama, OUTPUT);
       pinMode(Vav_Rca, OUTPUT);

       //SETEO LOS PIN COMO SALIDAS DE SONIDO
       pinMode(bocina_CA, OUTPUT);
       pinMode(bocina_AV, OUTPUT);

       //SETEO EL PIN COMO SALIDA DE ALUMBRADO
       pinMode(luz,OUTPUT);
       pinMode(sensor_ldr,INPUT);

       //INICIO LOS SEMAFOROS
       digitalWrite(Rav_Vca, LOW);
       digitalWrite(ama, LOW);
       digitalWrite(Vav_Rca, HIGH);

       //INICIO LAS BOCINAS
       digitalWrite(bocina_CA, LOW);
       digitalWrite(bocina_AV, LOW);

       //INICIO EL ALUMBRADO
       digitalWrite(luz,LOW);
       
       //INTERRUPCIONES
      attachInterrupt(0, atender , FALLING);   
            
}

void loop() {
    //Distancia del objeto al sensor
     long sensor1, sensor2;
    
    //Estas varibles se usan para determinar si paso el tiempo el cual debe esperar un auto para cruzar
    //Se hace con la funcion millis para no dejar al procesador ocioso, de esta manera mientras espero el tiempo determinado puedo ejecutar otras instrucciones
     unsigned long t_inicial, t_final;

     VerificarEmergencia();
     VerificarAlumbrado(); // esta lectura se hace esporadicamente debido a que el sol cae muy lentamente, no tiene sentido preguntar en tiempos de maquina la cantidad de luz solar.
    
     sensor1 = TomarDatosSensor(emisor_s1, receptor_s1);
     
     t_inicial = millis();
     
     while(HayAuto(sensor1)){
           
          VerificarEmergencia();
          
          EmitirSonidoLento(bocina_AV);
          EmitirSonidoRap(bocina_CA);
          
          if (digitalRead(Rav_Vca) == HIGH)                         // Sin este if, el verde de la calle quedara permanentemente prendido
            break;
            
          sensor2 = TomarDatosSensor(emisor_s2, receptor_s2);
          
          t_final = millis();
          
          if(HayAuto(sensor2)){                                    //Si estan activos los 2 sensores abre el semaforo de la calle para que crucen la avenida
              AbrirCalle(TIEMPO_PASO_CA_LA);
              BocinasLow();
          }
          else{                                                   //Si solo esta activo solo el primer sensor el auto debe esperar el tiempo determinado para cruzar la avenida
              if(ExcedioEspera(t_inicial, t_final)){
                  AbrirCalle(TIEMPO_PASO_CA_CO);
                  BocinasLow();
          }
          
          }
          sensor1 = TomarDatosSensor(emisor_s1, receptor_s1);
     }
     
     if(digitalRead(Rav_Vca) == HIGH){ // Si el rojo de la avenida esta encendido significa q estan habilitados a cruzar los autos de la calle. Entonces, se abre la avenida dandole el tiempo determinado para que crucen
        AbrirAvenida(TIEMPO_PASO_AV);
        BocinasLow();
     }
     else{                            /// Si el rojo de la avenida esta apagado significa ya estaban habilidatos de antes para cruzar, entonces no se les respeta el tiempo si hay autos en la calle
        AbrirAvenida();
        if(hay_peaton){
           AbrirCalle(TIEMPO_PASO_PEATON);
           BocinasLow();
           hay_peaton = 0;
        }
          
        
     } 

     
}

//FUNCIONES DE SENSOR
long TomarDatosSensor(int emisor, int receptor){
     long duracion, distancia;
     digitalWrite(emisor, LOW);        // Nos aseguramos de que el trigger está desactivado
     delayMicroseconds(2);              // Para asegurarnos de que el trigger esta LOW
     digitalWrite(emisor, HIGH);       // Activamos el pulso de salida
     delayMicroseconds(10);             // Esperamos 10µs. El pulso sigue active este tiempo
     digitalWrite(emisor, LOW);        // Cortamos el pulso y a esperar el echo
     duracion = pulseIn(receptor, HIGH);
     distancia = duracion / 2 / 29.1  ;//Calculo para distancia en cm
     return distancia;
}

boolean HayAuto(long sensor){
  if(sensor < LIMITE)
    return true;
  else
    return false;
}

boolean ExcedioEspera(unsigned long t_ini, unsigned long t_fin){
    if(t_fin - t_ini > TIEMPO_ESPERA)
      return true;
    else
      return false;
}

void MostrarDatos(long distancia, int n_sensor){
  Serial.println( "Sensor " + String(n_sensor) + " : " + String(distancia) + " cm.") ; // Muestra por pantalla los datos del sensor
}

//FUNCIONES DE SEMAFORO
void CambiarSemaforo(const char queAbro){
if(queAbro == 'C'){
    if (digitalRead(Rav_Vca) == LOW){
      digitalWrite(Vav_Rca, LOW);
      digitalWrite(ama, HIGH);
      delay (500);
      digitalWrite(ama, LOW);
      delay (500);
      }   
    digitalWrite (Rav_Vca, HIGH);
    return; 
  }
  else{
    if(digitalRead(Vav_Rca) == LOW){
    digitalWrite(Rav_Vca, LOW);
    digitalWrite(ama, HIGH);
    delay(500);
    digitalWrite(ama, LOW);
    delay(500);
    }
    digitalWrite(Vav_Rca, HIGH);
    return;
    }
    
}

void AbrirAvenida(){
   CambiarSemaforo('A');
   EmitirSonidoLento(bocina_AV);
   EmitirSonidoRap(bocina_CA); 
   return;
}

void AbrirAvenida(int tiempo_de_paso){
  unsigned long t_ini, t_fin;
  t_ini = millis();
  t_fin = millis();
  while(t_fin - t_ini < tiempo_de_paso){
    if(digitalRead(Rav_Vca) == HIGH){
        CambiarSemaforo('A');
    }
    VerificarEmergencia();
    EmitirSonidoLento(bocina_AV);
    EmitirSonidoRap(bocina_CA);
    delay(20);                 //No tengo idea porque, pero sin este delay se hace cualquier cosa
    t_fin = millis();
  }
  return;
}

void AbrirCalle(int tiempo){
  unsigned long t_ini, t_fin;
  t_ini = millis();
  t_fin = millis();
  //Serial.println("abrio calle");
  while(t_fin - t_ini < tiempo){
    if(digitalRead(Vav_Rca) == HIGH){
        CambiarSemaforo('C');
    }
    VerificarEmergencia();
    EmitirSonidoLento(bocina_CA);
    EmitirSonidoRap(bocina_AV);
    delay(20);                 //No tengo idea porque, pero sin este delay se hace cualquier cosa
    t_fin = millis();
  }
  return;
}

//FUNCIONES DE BOCINAS
void EmitirSonidoLento(int nro_bocina){
    if(millis() > t_lento){
      if(digitalRead(nro_bocina) == HIGH && millis()*2 > t_lento ){
        digitalWrite(nro_bocina, LOW);
        t_lento = millis() + SONIDO_LENTO;
      }
      else
       digitalWrite(nro_bocina, HIGH);
    
    }
    
  return;
}

void EmitirSonidoRap(int nro_bocina){
    if(millis() > t_rap){
      if(digitalRead(nro_bocina) == HIGH && millis()*2 > t_rap ){
        digitalWrite(nro_bocina, LOW);
        t_rap = millis() + SONIDO_RAP;
      }
      else
       digitalWrite(nro_bocina, HIGH);
    
    }
    
  return;
}
  
void BocinasLow(){
    digitalWrite(bocina_CA, LOW);
    digitalWrite(bocina_AV, LOW);
    return;
}

//FUNCIONES DE INTERRUPCION
void atender(){
    hay_peaton = 1;
}

//FUNCIONES DE EMERGENCIA (APP BLUETOOTH)
void LeerBuffer(){
  datoBT = Serial.read();
}

void Actuar(){
  if(datoBT == 'a')
      AbrirAvenida(TIEMPO_EMERGENCIA);
  if(datoBT == 'c')
      AbrirCalle(TIEMPO_EMERGENCIA);
}

void VerificarEmergencia(){
   if(Serial.available()){
     LeerBuffer();
     Actuar();
     Serial.flush();
   }
}

//FUNCIONES DE ALUMBRADO
//Un poco de teoria, cuando el ldr recibe mas luz, disminuye su resistencia. Si disminuye su resistencia la d.d.p. es menor entonces la arduino lee un valor mayor

void VerificarAlumbrado(){
 valor_ldr = analogRead(sensor_ldr);                    //Leo el valor del pin analogico
 //Serial.println(valor_ldr);
   if (valor_ldr < LIMITE_LUZ && digitalRead(luz) == LOW)     //Si es menor al limite y esta apagado
      digitalWrite(luz,HIGH);                                 //Enciendo el alumbrado publico
   if(valor_ldr >= LIMITE_LUZ && digitalRead(luz) == HIGH)   //Si es menor al limite y esta prendido
      digitalWrite(luz,LOW);                                 //Apago el alumbrado 
}
