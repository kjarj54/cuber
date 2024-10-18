# Proyecto de Simulación de Grafos en C++

## Descripción

Este proyecto consiste en la creación de un programa en C++ para la simulación de grafos tanto dirigidos como no dirigidos. El programa se basa en una aplicación de venta de servicios de transporte, representando gráficamente una ciudad y aplicando algoritmos basados en grafos para determinar la mejor ruta a distintos lugares.

## Funcionalidades

- **Mapa de la Ciudad**: Se muestra un mapa del Centro de San Isidro de Pérez Zeledón, con las vías representadas como un grafo.
- **Simulación de Vehículo**: Un vehículo se desplaza de un punto A a un punto B en el mapa, siguiendo la mejor ruta determinada por un algoritmo.
- **Selección de Algoritmo**: El usuario puede seleccionar entre diferentes algoritmos de grafos para calcular la mejor ruta.
- **Interacción en Tiempo Real**: El usuario puede seleccionar nodos y ver la ruta resaltada en pantalla. El vehículo se mueve por el mapa respetando los sentidos de las calles en grafos dirigidos.
- **Pesos de Aristas Dinámicos**: Las aristas del grafo tienen pesos proporcionales a la longitud de las vías en la realidad, y estos pesos pueden cambiar en tiempo de ejecución debido a factores como tráfico, accidentes o mantenimiento.
- **Visualización de Rutas**: Se muestran tanto la ruta original como la ruta ajustada en tiempo real si cambian las condiciones del tráfico.
- **Cálculo de Costos**: Al finalizar el recorrido, se presenta el costo asociado al transporte, considerando variables como tiempo y peso (longitud de las aristas).

## Algoritmos Implementados

- **Dijkstra**: Calcula el camino más corto desde un nodo inicial a todos los demás nodos, utilizando en pantalla el camino más corto entre dos nodos seleccionados.
- **Floyd-Warshall**: Construye una matriz de costos mínimos y caminos, utilizando el camino más corto para llevar el vehículo del nodo A al nodo B.

## Estructura del Proyecto
```
└── 📁cuber
    └── 📁cuber
        └── arial.ttf
        └── cuber.vcxproj
        └── cuber.vcxproj.filters
        └── cuber.vcxproj.user
        └── Dijkstra.cpp
        └── Dijkstra.h
        └── Floyd.cpp
        └── Floyd.h
        └── Fondo.png
        └── Graph.cpp
        └── Graph.h
        └── Node.cpp
        └── Node.h
        └── puntos.txt
        └── Source.cpp
        └── vertices.txt
    └── .gitignore
    └── cuber.sln
    └── readme
```
## Instrucciones de Ejecución

1. Clona el repositorio en tu máquina local.
2. Abre el proyecto en Visual Studio.
3. Asegúrate de que los archivos `arial.ttf`, `Fondo.png`, `puntos.txt` y `vertices.txt` estén en el directorio `cuber/`.
4. Compila y ejecuta el proyecto desde Visual Studio.
5. Sigue las instrucciones en pantalla para seleccionar los nodos y el algoritmo a aplicar.

## Autores

- [Kevin A]
- [Joksan M]
- [Kevin F]

## Licencia

Este proyecto está bajo la Licencia MIT.