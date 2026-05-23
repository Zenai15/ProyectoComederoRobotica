#include <LiquidCrystal.h>
#include <Servo.h>

//Indicamos los pines en los que está conectado el LCD
LiquidCrystal lcd(13, 12, 2, 3, 4, 5);

//Damos nombre al dispensador
Servo dispenser;

//Inicializamos las variables

//Se usan para mapear el valor analógico de los sensores de peso
int gato = 0;
int comida = 0;

//Variable para seleccionar en que modo está el programa: 0 --> Dispensar, 1 --> Seleccion de peso por ración, 2 -->Selección de intervalo de dispensación
int modo = 0;

//Contador para contar el tiempo sin que se pause el programa
int temporizador = 0;

//Valor predeterminado del tiempo de espera entre dispensaciones de la comida -- Ajustable en el programa
int tiempoDeEspera = 3600;

//Valor predeterminado del peso del pienso (Son 25, pero como el minimo del sensor son 20, se le añaden esos 20) -- Ajustable en el programa
int pesoLimiteComida = 45;

//Booleans con sus valores predeterminados para comprobar si se puede servir
bool servir = true;
bool horaDeServir = true;
bool hayGato = false;

//Declaracion de los pines de los componentes
const int pinPlato = A0;
const int pinGato = A1;
const int pinBotonConfirm = 8;
const int pinBotonSumar = 7;
const int pinBotonRestar = 6;

void setup() {
  Serial.begin(9600);

  //Indicamos el pin del servo y lo ponemos en su posicion inicial
  dispenser.attach(9);
  dispenser.write(0);

  //Ponemos los botones en modo resistencia pullup para que marquen cuando estan pulsados
  pinMode(pinBotonConfirm, INPUT_PULLUP);
  pinMode(pinBotonSumar, INPUT_PULLUP);
  pinMode(pinBotonRestar, INPUT_PULLUP);

  //Inicializamos el LCD con 16 caracteres y 2 filas
  lcd.begin(16, 2);
}


void loop() {
  //Comprobación de modo
  if (modo == 0) {
    //Comprobar si se está pulsando el boton de cambio de modo, si es el caso, cambiar al siguiente y limpiar la pantalla
    if (digitalRead(pinBotonConfirm) == LOW) {
      modo = 1;
      lcd.clear();
      delay(300);
    }
    //Mapeamos los valores analogicos de los sensores de peso para la comida y el gato: de 20 gramos (min del sensor) a 2000g (max del sensor)
    comida = map(analogRead(pinPlato), 0, 1000, 20, 2000);  //1000 es el maximo que comprobamos para este sensor
    gato = map(analogRead(pinGato), 0, 900, 20, 2000);      //900 es el máximo que comprobamos para este sensor

    //Limpiamos la pantalla del sensor en caso de que haya algo escrito
    lcd.setCursor(0, 0);  //Ponemos el cursor en la pos 0,0 para escribir al principio

    //Escribimos "GATO" y mostramos el peso del gato en gramos
    lcd.print("Gato:");
    lcd.print(gato);
    lcd.print("g   ");

    //Cambiamos a la siguiente fila y mostramos el peso de la comida también
    lcd.setCursor(0, 1);
    lcd.print("Comida:");
    lcd.print(comida);
    lcd.print("g   ");

    //Guardamos en el bool "hayGato" si hay peso en el sensor del gato o no
    hayGato = gato > 1000;

    //Guardamos un bool en el que se comprueba si no está en tiempo de espera, está el gato pidiendo comer y el bowl no está lleno
    servir = horaDeServir && hayGato && comida < 500;

    //Iniciamos un bucle que se reproduce hasta que el bowl esté lleno, siempre y cuando se cumpla el boolean servir
    while (servir && comida < pesoLimiteComida) {
      //Comprobamos de nuevo dentro del bucle si se pulsa el botón, para evitar quedarnos atascados
      if (digitalRead(pinBotonConfirm) == LOW) {
        modo = 1;
        lcd.clear();
        delay(300);
        break;
      }
      //Volvemos a ejecutar los print y lecturas por pantalla hasta que termine de echar comida
      //Si no se vuelven a ejecutar, el programa sigue echando comida sin leer el peso, por lo que se vuelve infinito
      comida = map(analogRead(pinPlato), 0, 950, 0, 2000);
      gato = map(analogRead(pinGato), 0, 900, 0, 2000);

      //Escribimos en el LCD el peso del gato y de la comida
      lcd.setCursor(0, 0);
      lcd.print("Gato:");
      lcd.print(gato);
      lcd.print("g   ");

      lcd.setCursor(0, 1);
      lcd.print("Comida:");
      lcd.print(comida);
      lcd.print("g   ");

      //Abrimos y cerramos el dispensador por tandas para poder calcular si la comida llega al peso limite guardado
      dispenser.write(90);
      delay(250);
      dispenser.write(0);
      delay(1000);

      //Comprueba si la comida supera la toma guardada
      if (comida > pesoLimiteComida) {
        horaDeServir = false;
      }
    }
    //Espera de 1 segundo entre mediciones para asegurar la funcionalidad del temporizador y no sobrecargar el programa
    delay(1000);

    //Añadimos 1 al temporizador cada segundo
    temporizador += 1;

    //Se comprueba si ha pasado el tiempo de espera sin necesidad de parar el programa
    if (temporizador >= tiempoDeEspera) {

      //Si el temporizador alcanza el tiempo de espera, se reinicia para volver a contarlo
      temporizador = 0;

      //Pone el boolean de la hora de servir a true para que el bool "servir" pueda ser true
      horaDeServir = true;
    }
  //Cambio de modo
  } else if (modo == 1) {
    
    //Escribimos por pantalla el peso por ración actual, que se verá cambiado al modificarlo
    lcd.setCursor(0, 0);
    lcd.print("Peso por racion:");
    lcd.setCursor(0,1);
    lcd.print(pesoLimiteComida - 20);
    lcd.print("g                     ");
    
    //Utilizamos los botones para sumar o restar al limite de peso por ración
    if (digitalRead(pinBotonRestar) == LOW) {
      if (pesoLimiteComida >= 1) pesoLimiteComida -= 1;
      delay(150);
    } else if (digitalRead(pinBotonSumar) == LOW) {
      pesoLimiteComida += 1;
      delay(150);
    }

    //Botón de confirmacion para cambiar de modo y terminar el ajuste de comida
    if (digitalRead(pinBotonConfirm) == LOW) {
      modo = 2;
      lcd.clear();
      delay(300);
    }

  //Cambio de modo
  } else if (modo == 2) {

    //Imprimimos por pantalla el tiempo de espera en minutos
    lcd.setCursor(0, 0);
    lcd.print("Intervalo:");
    lcd.setCursor(0,1);
    lcd.print(tiempoDeEspera / 60);
    lcd.print(" mins   ");
    

    //Usamos los botones para sumar o restar tiempo de 30 en 30 minutos
    if (digitalRead(pinBotonRestar) == LOW) {
      if(tiempoDeEspera >= 1800) tiempoDeEspera -= 1800;
      delay(200);
    } else if (digitalRead(pinBotonSumar) == LOW) {
      tiempoDeEspera += 1800;
      delay(200);
    }

    //Volver al modo de dispensación
    if (digitalRead(pinBotonConfirm) == LOW) {
      modo = 0;
      lcd.clear();
      delay(300);
    }

  //En caso de error del boton, volvería al menu principar igualmente
  } else {
    modo = 0;
  }
}