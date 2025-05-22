cplusplus {{
    /** RSU message type */
    static const int RSU_TYPE = 12340;
}}

packet RsuMessage {
    int rsuId;
    int sequenceNumber;
    int destinationId;
}