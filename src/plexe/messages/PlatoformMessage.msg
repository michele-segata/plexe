cplusplus {{
        /** message type for PLATOFORM messages */
        static const int PLATOFORM_TYPE = 12349;
    }}


packet PlatoformMessage {
    int vehicleId;
    int platoonId;
    int destinationId;
}