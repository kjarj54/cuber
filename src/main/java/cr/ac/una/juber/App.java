package cr.ac.una.juber;

import cr.ac.una.juber.util.FlowController;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.io.IOException;

/**
 * JavaFX App
 */
public class App extends Application {

    private static Scene scene;

    @Override
    public void start(Stage stage) throws IOException {
        FlowController.getInstance().InitializeFlow(stage,null);
        stage.setTitle("Juber");
        FlowController.getInstance().goViewInWindow("HelloWorld");
    }
    
    public static void main(String[] args) {
        launch();
    }

}