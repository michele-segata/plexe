cplusplus {{
        /** message type for PLATOFORM messages */
        static const int PF_TYPE = 12350;
    }}


packet PFMessage {
    unsigned int sessionId;
    int vehicleId;
}