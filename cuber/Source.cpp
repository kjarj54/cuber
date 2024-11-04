#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Dijkstra.h"
#include "Floyd.h"
#include <string>
#include "Incident.cpp"

using namespace std;

std::vector<Incident> incidents;


void loadVerticesFromFile(Graph& graph, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        int id;
        float x, y;
        char dot;
        if (!(iss >> id >> dot >> x >> y)) { break; } // Error al leer la línea
        string nodeName = "Node" + to_string(id);
        graph.addNode(nodeName, x, y);
    }
    file.close();
}

void loadGraphFromFile(Graph& graph, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.find("Punto") != string::npos) {
            int id;
            sscanf_s(line.c_str(), "Punto %d:", &id);
            string nodeName = "Node" + to_string(id);

            // Leer vecinos
            getline(file, line);
            istringstream iss(line.substr(line.find(":") + 1));
            string neighbor;
            while (getline(iss, neighbor, ',')) {
                neighbor.erase(remove(neighbor.begin(), neighbor.end(), '['), neighbor.end());
                neighbor.erase(remove(neighbor.begin(), neighbor.end(), ']'), neighbor.end());
                neighbor.erase(remove(neighbor.begin(), neighbor.end(), ' '), neighbor.end());
                int neighborId = stoi(neighbor);
                string neighborName = "Node" + to_string(neighborId);
                graph.addEdge(nodeName, neighborName, 1.0); // Placeholder weight
            }
        }
    }
    file.close();
}

void drawGraph(sf::RenderWindow& window, Graph& graph, sf::Font& font) {
    for ( auto& vertex : graph.getVertices()) {
        Node* node = graph.getNode(vertex);
        auto neighbors = graph.getNeighbors(vertex);

        // Dibujar el nodo
        sf::CircleShape shape(10); // Radio del círculo más grande
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(node->getX(), node->getY());
        window.draw(shape);

        // Dibujar el ID del nodo
        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(std::stoi(node->getId().substr(4)))); // Extraer el ID numérico
        text.setCharacterSize(14); // Tamaño del texto
        text.setFillColor(sf::Color::Black);
        text.setPosition(node->getX(), node->getY());
        window.draw(text);

        for ( auto& neighbor : neighbors) {
            Node* neighborNode = graph.getNode(neighbor.first);

            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(node->getX(), node->getY()), sf::Color::Red),
                sf::Vertex(sf::Vector2f(neighborNode->getX(), neighborNode->getY()), sf::Color::Red)
            };

            window.draw(line, 2, sf::Lines);

            if (graph.isDirected()) {
                // Dibujar una segunda línea para los bordes bidireccionales
                sf::Vertex reverseLine[] = {
                    sf::Vertex(sf::Vector2f(neighborNode->getX(), neighborNode->getY()), sf::Color::Blue),
                    sf::Vertex(sf::Vector2f(node->getX(), node->getY()), sf::Color::Blue)
                };

                window.draw(reverseLine, 2, sf::Lines);
            }
        }
    }
}


// Función para abrir la ventana de incidentes y añadir uno nuevo
void openIncidentWindow(Graph& graph) {
    sf::RenderWindow incidentWindow(sf::VideoMode(400, 400), "Reportar Incidente", sf::Style::Titlebar | sf::Style::Close);

    bool isCoseviSelected = false;
    bool isAccidentSelected = false;
    bool showDirectionOptions = false;
    string selectedDirection = "Ambas Direcciones";
    bool isFromSelected = false;
    bool isToSelected = false;
    string toStr;
    string fromStr;

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente arial.ttf." << std::endl;
        return;
    }
    // Configuración de los campos de texto "Desde Punto" y "Hasta Punto"
    sf::Text fromLabel("Desde Punto:", font, 14);
    fromLabel.setPosition(20, 50);
    fromLabel.setFillColor(sf::Color::Black);

    sf::RectangleShape fromBox(sf::Vector2f(100, 25));
    fromBox.setPosition(120, 50);
    fromBox.setFillColor(sf::Color(220, 220, 220));
    fromBox.setOutlineColor(sf::Color::Black);
    fromBox.setOutlineThickness(1);

    sf::Text fromText("", font, 14);
    fromText.setPosition(125, 53);
    fromText.setFillColor(sf::Color::Black);
    

    sf::Text toLabel("Hasta Punto:", font, 14);
    toLabel.setPosition(20, 90);
    toLabel.setFillColor(sf::Color::Black);

    sf::RectangleShape toBox(sf::Vector2f(100, 25));
    toBox.setPosition(120, 90);
    toBox.setFillColor(sf::Color(220, 220, 220));
    toBox.setOutlineColor(sf::Color::Black);
    toBox.setOutlineThickness(1);

    sf::Text toText("", font, 14);
    toText.setPosition(125, 93);
    toText.setFillColor(sf::Color::Black);
    

    // Configuración de los botones para tipo de incidente
    sf::Text incidentTypeLabel("Tipo de Incidente:", font, 14);
    incidentTypeLabel.setPosition(20, 150);
    incidentTypeLabel.setFillColor(sf::Color::Black);

    sf::RectangleShape coseviButton(sf::Vector2f(200, 30));
    coseviButton.setPosition(20, 180);
    coseviButton.setFillColor(sf::Color::Blue);

    sf::Text coseviText("Calle cerrada por mantenimiento (COSEVI)", font, 14);
    coseviText.setPosition(25, 185);
    coseviText.setFillColor(sf::Color::White);

    sf::RectangleShape accidentButton(sf::Vector2f(200, 30));
    accidentButton.setPosition(20, 220);
    accidentButton.setFillColor(sf::Color::Green);

    sf::Text accidentText("Accidente", font, 14);
    accidentText.setPosition(25, 225);
    accidentText.setFillColor(sf::Color::White);

    // Botones de dirección
    sf::RectangleShape dir1Button(sf::Vector2f(150, 30));
    dir1Button.setPosition(20, 270);
    dir1Button.setFillColor(sf::Color::Cyan);

    sf::Text dir1Text("Dirección 1", font, 14);
    dir1Text.setPosition(30, 275);
    dir1Text.setFillColor(sf::Color::Black);

    sf::RectangleShape dir2Button(sf::Vector2f(150, 30));
    dir2Button.setPosition(200, 270);
    dir2Button.setFillColor(sf::Color::Cyan);

    sf::Text dir2Text("Dirección 2", font, 14);
    dir2Text.setPosition(210, 275);
    dir2Text.setFillColor(sf::Color::Black);

    // Botón de confirmación
    sf::RectangleShape confirmButton(sf::Vector2f(150, 30));
    confirmButton.setPosition(125, 330);
    confirmButton.setFillColor(sf::Color::Red);

    sf::Text confirmButtonText("Confirmar Incidente", font, 14);
    confirmButtonText.setPosition(130, 335);
    confirmButtonText.setFillColor(sf::Color::White);

    while (incidentWindow.isOpen()) {
        sf::Event event;
        while (incidentWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                incidentWindow.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (coseviButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isCoseviSelected = true;
                    isAccidentSelected = false;
                    // Verificar si ambos puntos son vecinos y si la calle es de doble sentido
                    if (!fromStr.empty() && !toStr.empty() && graph.isDirectNeighbor("Node" + fromStr, "Node" + toStr)) {
                        showDirectionOptions = graph.isDoubleWay("Node" + fromStr, "Node" + toStr);
                    }
                    else {
                        showDirectionOptions = false;
                    }
                    coseviButton.setFillColor(sf::Color::Yellow);
                    accidentButton.setFillColor(sf::Color::Green);
                }
                else if (accidentButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isAccidentSelected = true;
                    isCoseviSelected = false;

                    // Verificar si ambos puntos son vecinos y si la calle es de doble sentido
                    if (!fromStr.empty() && !toStr.empty() && graph.isDirectNeighbor("Node" + fromStr, "Node" + toStr)) {
                        showDirectionOptions = graph.isDoubleWay("Node" + fromStr, "Node" + toStr);
                    }
                    else {
                        showDirectionOptions = false;
                    }

                    accidentButton.setFillColor(sf::Color::Yellow);
                    coseviButton.setFillColor(sf::Color::Blue);
                }

                if (fromBox.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isFromSelected = true;
                    isToSelected = false;
                    fromBox.setOutlineColor(sf::Color::Blue);
                    toBox.setOutlineColor(sf::Color::Black);
                }
                else if (toBox.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isFromSelected = false;
                    isToSelected = true;
                    fromBox.setOutlineColor(sf::Color::Black);
                    toBox.setOutlineColor(sf::Color::Blue);
                }
                else {
                    isFromSelected = false;
                    isToSelected = false;
                    fromBox.setOutlineColor(sf::Color::Black);
                    toBox.setOutlineColor(sf::Color::Black);
                }

                if (showDirectionOptions && dir1Button.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    selectedDirection = "Direccion 1";
                    dir1Button.setFillColor(sf::Color::Yellow);
                    dir2Button.setFillColor(sf::Color::Cyan);
                }
                else if (showDirectionOptions && dir2Button.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    selectedDirection = "Direccion 2";
                    dir2Button.setFillColor(sf::Color::Yellow);
                    dir1Button.setFillColor(sf::Color::Cyan);
                }

                if (confirmButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    string fromNodeID = "Node" + fromStr;
                    string toNodeID = "Node" + toStr;

                    if (!graph.isDirectNeighbor(fromNodeID, toNodeID)) {
                        std::cerr << "Error: Los puntos no están conectados directamente." << std::endl;
                        continue;
                    }

                    Incident newIncident;
                    newIncident.fromPoint = fromNodeID;
                    newIncident.toPoint = toNodeID;
                    newIncident.type = isCoseviSelected ? "COSEVI" : "Accidente";
                    newIncident.direction = showDirectionOptions ? selectedDirection : "Ambas Direcciones";
                    incidents.push_back(newIncident);
                    incidentWindow.close();
                }
            }

            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char enteredChar = static_cast<char>(event.text.unicode);
                    if (std::isdigit(enteredChar)) {
                        if (isFromSelected && fromStr.length() < 3) {
                            fromStr += enteredChar;
                        }
                        else if (isToSelected && toStr.length() < 3) {
                            toStr += enteredChar;
                        }
                    }
                    else if (event.text.unicode == '\b') {
                        if (isFromSelected && !fromStr.empty()) {
                            fromStr.pop_back();
                        }
                        else if (isToSelected && !toStr.empty()) {
                            toStr.pop_back();
                        }
                    }
                    fromText.setString(fromStr);
                    toText.setString(toStr);
                }
            }
        }

        incidentWindow.clear(sf::Color::White);
        incidentWindow.draw(fromLabel);
        incidentWindow.draw(fromBox);
        incidentWindow.draw(fromText);
        incidentWindow.draw(toLabel);
        incidentWindow.draw(toBox);
        incidentWindow.draw(toText);
        incidentWindow.draw(incidentTypeLabel);
        incidentWindow.draw(coseviButton);
        incidentWindow.draw(coseviText);
        incidentWindow.draw(accidentButton);
        incidentWindow.draw(accidentText);
        
        if (showDirectionOptions) {
            incidentWindow.draw(dir1Button);
            incidentWindow.draw(dir1Text);
            incidentWindow.draw(dir2Button);
            incidentWindow.draw(dir2Text);
        }

        incidentWindow.draw(confirmButtonText);
        incidentWindow.draw(confirmButton);
        
        incidentWindow.display();
    }
}

// Pantalla para seleccionar puntos A y B, algoritmo y listar incidentes
void openMenuWindow(Graph& graph) {
    sf::RenderWindow menuWindow(sf::VideoMode(500, 400), "Menu", sf::Style::Titlebar | sf::Style::Close);

    //variables
    string inputStrA;
    string inputStrB;
    bool isInputASelected = false;
    bool isInputBSelected = false;


    // Cargar la fuente
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente arial.ttf." << std::endl;
        return;
    }

    // Texto para Punto A y Punto B
    sf::Text labelA("Punto A:", font, 14);
    labelA.setPosition(20, 20);
    labelA.setFillColor(sf::Color::Black);

    sf::Text labelB("Punto B:", font, 14);
    labelB.setPosition(20, 60);
    labelB.setFillColor(sf::Color::Black);

    // Fondo de los campos de entrada
    sf::RectangleShape inputBoxA(sf::Vector2f(100, 25));
    inputBoxA.setPosition(100, 20);
    inputBoxA.setFillColor(sf::Color(220, 220, 220));  // Gris claro
    inputBoxA.setOutlineColor(sf::Color::Black);
    inputBoxA.setOutlineThickness(1);

    sf::RectangleShape inputBoxB(sf::Vector2f(100, 25));
    inputBoxB.setPosition(100, 60);
    inputBoxB.setFillColor(sf::Color(220, 220, 220));  // Gris claro
    inputBoxB.setOutlineColor(sf::Color::Black);
    inputBoxB.setOutlineThickness(1);

    // Campos de texto para mostrar los valores de entrada
    sf::Text inputA("", font, 14);
    inputA.setPosition(105, 23);  // Posición ajustada para centrar el texto dentro del cuadro
    inputA.setFillColor(sf::Color::Black);
    

    sf::Text inputB("", font, 14);
    inputB.setPosition(105, 63);  // Posición ajustada para centrar el texto dentro del cuadro
    inputB.setFillColor(sf::Color::Black);

    // Botones de selección de algoritmo
    sf::RectangleShape floydButton(sf::Vector2f(120, 30));
    floydButton.setPosition(20, 100);
    floydButton.setFillColor(sf::Color::Blue);

    sf::Text floydText("Floyd", font, 14);
    floydText.setPosition(30, 105);
    floydText.setFillColor(sf::Color::Black);

    sf::RectangleShape dijkstraButton(sf::Vector2f(120, 30));
    dijkstraButton.setPosition(160, 100);
    dijkstraButton.setFillColor(sf::Color::Green);

    sf::Text dijkstraText("Dijkstra", font, 14);
    dijkstraText.setPosition(170, 105);
    dijkstraText.setFillColor(sf::Color::Black);

    bool useFloyd = false;

    // Botón para crear lista de incidentes
    sf::RectangleShape incidentButton(sf::Vector2f(200, 40));
    incidentButton.setPosition(100, 200);
    incidentButton.setFillColor(sf::Color::Red);

    sf::Text incidentButtonText("Listar Calles con Incidentes", font, 14);
    incidentButtonText.setPosition(105, 210);
    incidentButtonText.setFillColor(sf::Color::White);

    sf::Text incidentListLabel("Lista de Incidentes:", font, 14);
    incidentListLabel.setPosition(20, 150);
    incidentListLabel.setFillColor(sf::Color::Black);

    while (menuWindow.isOpen()) {
        sf::Event event;
        while (menuWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                menuWindow.close();

            // Detección de selección de algoritmo
            if (event.type == sf::Event::MouseButtonPressed) {
                if (floydButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    useFloyd = true;
                    floydButton.setFillColor(sf::Color::Yellow);
                    dijkstraButton.setFillColor(sf::Color::Green);
                }
                else if (dijkstraButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    useFloyd = false;
                    dijkstraButton.setFillColor(sf::Color::Yellow);
                    floydButton.setFillColor(sf::Color::Blue);
                }

                if (incidentButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    openIncidentWindow(graph);
                }

                // Selección de campo de texto
                if (inputBoxA.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isInputASelected = true;
                    isInputBSelected = false;
                    inputBoxA.setOutlineColor(sf::Color::Blue);
                    inputBoxB.setOutlineColor(sf::Color::Black);
                }
                else if (inputBoxB.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isInputASelected = false;
                    isInputBSelected = true;
                    inputBoxA.setOutlineColor(sf::Color::Black);
                    inputBoxB.setOutlineColor(sf::Color::Blue);
                }
                else {
                    // Desactivar ambos campos si se hace clic fuera de ellos
                    isInputASelected = false;
                    isInputBSelected = false;
                    inputBoxA.setOutlineColor(sf::Color::Black);
                    inputBoxB.setOutlineColor(sf::Color::Black);
                }
            }

            // Captura de texto para los puntos A y B
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    char enteredChar = static_cast<char>(event.text.unicode);
                    if (std::isdigit(enteredChar)) {
                        if (isInputASelected && inputStrA.length() < 3) {
                            inputStrA += enteredChar;  // Añadir carácter al campo A
                        }
                        else if (isInputBSelected && inputStrB.length() < 3) {
                            inputStrB += enteredChar;  // Añadir carácter al campo B
                        }
                    }
                    else if (event.text.unicode == '\b') {  // Detectar retroceso
                        if (isInputASelected && !inputStrA.empty()) {
                            inputStrA.pop_back();  // Borrar último carácter de A
                        }
                        else if (isInputBSelected && !inputStrB.empty()) {
                            inputStrB.pop_back();  // Borrar último carácter de B
                        }
                    }
                    inputA.setString(inputStrA);
                    inputB.setString(inputStrB);
                }
            }
        }

        menuWindow.clear(sf::Color::White);  // Fondo blanco

        // Dibujar etiquetas y cajas de entrada
        menuWindow.draw(labelA);
        menuWindow.draw(inputBoxA);
        menuWindow.draw(inputA);

        menuWindow.draw(labelB);
        menuWindow.draw(inputBoxB);
        menuWindow.draw(inputB);

        // Dibujar botones de selección de algoritmo y sus textos
        menuWindow.draw(floydButton);
        menuWindow.draw(floydText);
        menuWindow.draw(dijkstraButton);
        menuWindow.draw(dijkstraText);

        // Dibujar botón de incidentes y su texto
        menuWindow.draw(incidentButton);
        menuWindow.draw(incidentButtonText);

        menuWindow.draw(incidentListLabel);

        // Mostrar incidentes
        float yPosition = 180;
        for (size_t i = 0; i < incidents.size(); ++i) {
            sf::Text incidentText("Incidente " + std::to_string(i + 1) + ": " + incidents[i].type +
                " desde " + incidents[i].fromPoint + " hasta " + incidents[i].toPoint,
                font, 12);
            incidentText.setPosition(20, yPosition);
            incidentText.setFillColor(sf::Color::Black);
            menuWindow.draw(incidentText);

            yPosition += 20;
        }

        menuWindow.display();
    }
}
void drawPath(sf::RenderWindow& window, Graph& graph, vector<string>& path) {
    for (size_t i = 0; i < path.size() - 1; ++i) {
        Node* node = graph.getNode(path[i]);
        Node* nextNode = graph.getNode(path[i + 1]);

        // Calcular la posición y el ángulo de la línea
        sf::Vector2f start(node->getX(), node->getY());
        sf::Vector2f end(nextNode->getX(), nextNode->getY());
        sf::Vector2f direction = end - start;

        // Calcular la longitud de la línea
        float length = sqrt(direction.x * direction.x + direction.y * direction.y);

        // Crear un rectángulo para representar la línea ancha
        sf::RectangleShape line(sf::Vector2f(length, 5)); // Ajusta el segundo parámetro para el grosor deseado
        line.setFillColor(sf::Color::Green);

        // Establecer la posición y rotación
        line.setPosition(start);
        line.setRotation(atan2(direction.y, direction.x) * 180 / 3.14159265f); // Convierte de radianes a grados

        window.draw(line);
    }
}





int main()
{
    sf::RenderWindow window;

    // Cargar la imagen de fondo
    sf::Texture texture;
    if (!texture.loadFromFile("Fondo.png"))
    {
        std::cerr << "Error: No se pudo cargar la imagen desde el archivo." << std::endl;
        return -1;
    }

    sf::Vector2u textureSize = texture.getSize();

    // Definir el tamaño fijo de la ventana (puede ser 1280x720 o lo que prefieras)
    const unsigned int fixedWidth = 1280;
    const unsigned int fixedHeight = 720;

    // Crear la ventana con el tamaño fijo
    window.create(sf::VideoMode(fixedWidth, fixedHeight), "SFML Fixed Size Window", sf::Style::Close);

    sf::Sprite sprite;
    sprite.setTexture(texture);
    // Escalar la imagen de fondo para que se ajuste al tamaño de la ventana
    sprite.setScale(
        static_cast<float>(fixedWidth) / textureSize.x,
        static_cast<float>(fixedHeight) / textureSize.y
    );

    Graph graph(true); // true para grafo dirigido

    // Leer los vértices desde el archivo vertices.txt
    loadVerticesFromFile(graph, "vertices.txt");

    // Leer los vecinos desde el archivo puntos.txt
    loadGraphFromFile(graph, "puntos.txt");

    // Crear una instancia de Dijkstra y calcular la ruta
    Dijkstra dijkstra(&graph);
    string startNode = "Node1"; // Nodo inicial (cámbialo según tus necesidades)
    string endNode = "Node10";  // Nodo final (cámbialo según tus necesidades)
    vector<string> path = dijkstra.shortestPath(startNode, endNode);

    // Cargar la fuente para los números
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Error: No se pudo cargar la fuente arial.ttf." << endl;
        return -1;
    }

    // Crear botón para abrir el menú
    sf::RectangleShape menuButton(sf::Vector2f(100, 50));
    menuButton.setFillColor(sf::Color::Green);
    menuButton.setPosition(fixedWidth - 110, 10); // Posición en la esquina superior derecha

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (menuButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    openMenuWindow(graph);
                }
            }
        }

        window.clear();
        window.draw(sprite);

        // Dibujar el grafo completo
        drawGraph(window, graph, font);

        // Dibujar la ruta específica en azul
        drawPath(window, graph, path);

        // Dibujar botón de menú
        window.draw(menuButton);

        window.display();
    }

    return 0;
}

