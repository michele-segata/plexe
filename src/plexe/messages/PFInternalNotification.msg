cplusplus {{
        /** message type for PLATOFORM messages */
        static const int PF_NOTIFICATION = 12351;
}}

packet PFInternalNotification {
    int sessionId;
    int notice;
}