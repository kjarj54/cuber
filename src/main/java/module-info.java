module cr.ac.una.juber {
    requires javafx.controls;
    requires javafx.fxml;

    opens cr.ac.una.juber to javafx.fxml;
    exports cr.ac.una.juber;
}
