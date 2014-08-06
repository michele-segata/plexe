packet ManeuverMessage {
	//id of the originator
	int vehicleId;
	//platoon id
	int platoonId;
	//message type
	int messageType;
	//platoon speed
	double platoonSpeed;
	//platoon lane
	int platoonLane;
	//id of vehicle in front of the joiner (G)
	int frontVehicleId;
	//id of the follower vehicle (F)
	int followerVehicleId;
	//platoon formation
	int platoonFormation[];
}
