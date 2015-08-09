#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	/* of Setup */
	ofSetVerticalSync(true);
	ofSetWindowShape(1920, 1080);
	ofSetFrameRate(60);

	/* Kinect Setup */
	kinect.open();
	kinect.initColorSource();
	kinect.initBodySource();
	kinect.initDepthSource();
	kinectScale = 1;
	camera.setDistance(1);	// MOVING THE CAMERA CLOSER

	/* Particle Setup */
	int num = 750;
	p1.assign(num, demoParticle());
	p2.assign(num, demoParticle());
	currentMode = PARTICLE_MODE_ATTRACT;

	currentModeStr = "1 - PARTICLE_MODE_ATTRACT: attracts to mouse"; 

	resetParticles();
}

//--------------------------------------------------------------
void ofApp::resetParticles(){

	//these are the attraction points used in the forth demo 
	p1AttractPoints.clear();
	for(int i = 0; i < 4; i++){
		p1AttractPoints.push_back( ofPoint( ofMap(i, 0, 4, 100, ofGetWidth()-100) , ofRandom(100, ofGetHeight()-100) ) );
	}
	
	p1AttractPointsWithMovement = p1AttractPoints;
	
	for(unsigned int i = 0; i <  p1.size(); i++){
		p1[i].setMode(currentMode);		
		p1[i].setAttractPoints(&p1AttractPointsWithMovement);;
		p1[i].reset();
		p2[i].setMode(currentMode);		
		p2[i].reset();
	}	
}

void ofApp::setHandCursor(JointType jointType, ofVec3f pos){
	
	float displayX = 0;
	float displayY = 0;

	
	CameraSpacePoint csp;
	ColorSpacePoint colorSpacePoint ={0.0f, 0.0f};
	csp.X = pos.x;
	csp.Y = pos.y;
	csp.Z = pos.z;
	
	ICoordinateMapper* mapper;
	HRESULT hresult = kinect.getSensor()->get_CoordinateMapper(&mapper);
	if(FAILED(hresult)){
		ofLog() << "404:Mapper Not Found";
	}
	
	//mapper->MapCameraPointsToColorSpace(1, &csp, 1, &colorSpacePoint);
	hresult = mapper->MapCameraPointToColorSpace(csp, &colorSpacePoint);
	if(FAILED(hresult)){
		ofLog() << "404:Mapper Not Found";
	}

	if( currentMode == PARTICLE_MODE_NEAREST_POINTS ) {
		
		if(jointType == JointType_HandLeft) 
			p1AttractPoints[0]= ofPoint(colorSpacePoint.X, colorSpacePoint.Y);
		if(jointType == JointType_HandRight)
			p1AttractPoints[1]= ofPoint(colorSpacePoint.X, colorSpacePoint.Y);
		if(jointType == JointType_FootRight) 
			p1AttractPoints[2]= ofPoint(colorSpacePoint.X, colorSpacePoint.Y);
		if(jointType == JointType_FootLeft)
			p1AttractPoints[3]= ofPoint(colorSpacePoint.X, colorSpacePoint.Y);
	}else if ( currentMode == PARTICLE_MODE_ATTRACT){
		if(jointType == JointType_HandLeft){
			for(int i =0; i<p1.size();i++){ 
				p1[i].setPosX(colorSpacePoint.X);
				p1[i].setPosY(colorSpacePoint.Y);
			}
		} 
		if(jointType == JointType_HandRight){
			for(int i =0; i<p2.size();i++){ 
				p2[i].setPosX(colorSpacePoint.X);
				p2[i].setPosY(colorSpacePoint.Y);
			}
		}
	}

	//Setting Particle to the Join Position
	
}

//--------------------------------------------------------------
void ofApp::update(){
	// UDPATE ALL INITIALIZED KINECT STREAMS
	kinect.update();

	for(unsigned int i = 0; i <  p1.size(); i++){
		p1[i].setMode(currentMode);
		p1[i].update();
		
	}
	for(unsigned int i = 0; i <  p2.size(); i++){
		p2[i].setMode(currentMode);
		p2[i].update();
	}

	//lets add a bit of movement to the attract points
	for(unsigned int i = 0; i < p1AttractPointsWithMovement.size(); i++){
		p1AttractPointsWithMovement[i].x = p1AttractPoints[i].x + ofSignedNoise(i * 10, ofGetElapsedTimef() * 0.7) * 12.0;
		p1AttractPointsWithMovement[i].y = p1AttractPoints[i].y + ofSignedNoise(i * -10, ofGetElapsedTimef() * 0.7) * 12.0;
	}	

}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor(60,60,60), ofColor(10,10,10));

	for(unsigned int i = 0; i < p1.size(); i++){
		p1[i].draw();
	}
	for(unsigned int i = 0; i < p2.size(); i++){
		p2[i].draw();
	}
	
	ofSetColor(190);
	if( currentMode == PARTICLE_MODE_NEAREST_POINTS ){
		for(unsigned int i = 0; i < p1AttractPoints.size(); i++){
			ofNoFill();
			ofCircle(p1AttractPointsWithMovement[i], 10);
			ofFill();
			ofCircle(p1AttractPointsWithMovement[i], 4);
		}
	}

	ofSetColor(230);	
	ofDrawBitmapString(currentModeStr + "\n\nSpacebar to reset. \nKeys 1-4 to change mode.", 10, 20);
	
	// WE ARE NOW SWITCHING TO THE 3D VIEW
	camera.begin();
	ofPushStyle();
	// FOR THIS MESH, I SCALED THE Z NEGATIVE TO MATCH DEPTH
	// THIS IS DEVELOPER PREFERENCE, BUT IT'S HOW I CONCEIVE
	// THE MESH...
	ofScale(10, 10, -10);

	// NOW DRAW OUR JOINTS IN THE SAME 3D SPACE
	drawJoints3D();
	ofPopStyle();
	camera.end();
}

void ofApp::drawJoints3D()
{
	// DRAW THE JOINTS IN A SALMON COLOR
	ofSetColor(240, 120, 90);

	ofVec3f pos;

	// THERE IS A MAXIMUM OF 6 BODIES TRACKED BY KINECT
	for (int i = 0; i<6; i++){
		// IF THE BODY IS BEING TRACKED...
		if (this->kinect.getBodySource()->getBodies()[i].tracked){

			auto b = this->kinect.getBodySource()->getBodies()[i];
			HandState leftHandState = b.leftHandState;
			ofColor * tmpColor;
			if(i == 0){
				tmpColor = new ofColor(50, 255, 50);
			}
			else if(i == 1){
				tmpColor = new ofColor(250, 255, 50);
			}
			else if(i == 2){
				tmpColor = new ofColor(0, 255, 0);
			}
			else if(i == 3){
				tmpColor = new ofColor(0, 0, 0);
			}
			else if(i == 4){
				tmpColor = new ofColor(25, 255, 0);
			}else
				tmpColor = new ofColor(34, 34, 34);

			// ITERATE THROUGH ALL JOINTS IN THE TRACKED BODY...
			for (std::map<JointType, ofxKFW2::Data::Joint>::iterator it = b.joints.begin(); it != b.joints.end(); ++it)
			{
				if (it->second.getTrackingState() == TrackingState_Tracked)
				{
					// GRAB THE JOINT'S 3D POSITION
					pos = it->second.getPosition();

					if(it->second.getType() == JointType_HandLeft){
						if(leftHandState == HandState::HandState_Closed)
							ofSetColor(255,0,0);
						setHandCursor(it->second.getType(), pos);
					}else
						ofSetColor(*tmpColor);
					
					if(it->second.getType() == JointType_HandRight)
						setHandCursor(it->second.getType(), pos);
					
					if(it->second.getType() == JointType_FootRight)
						setHandCursor(it->second.getType(), pos);
					
					if(it->second.getType() == JointType_FootLeft)
						setHandCursor(it->second.getType(), pos);

					// AND RENDER. JOINTS AND THE MESH POINTS SHARE THE 
					// SAME DEPTH SPACE COORDINATES SO NO COORDINATE MAPPING
					// IS NEEDED FOR THIS. 
					ofBox(pos.x*kinectScale, pos.y*kinectScale, pos.z*kinectScale, .01, .01, .01);
				}
			}
			delete tmpColor;
			tmpColor = NULL;
		}
	}
	ofSetColor(255);
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if( key == '1'){
		currentMode = PARTICLE_MODE_ATTRACT;
		currentModeStr = "1 - PARTICLE_MODE_ATTRACT: attracts to mouse"; 		
	}
	if( key == '2'){
		currentMode = PARTICLE_MODE_REPEL;
		currentModeStr = "2 - PARTICLE_MODE_REPEL: repels from mouse"; 				
	}
	if( key == '3'){
		currentMode = PARTICLE_MODE_NEAREST_POINTS;
		currentModeStr = "3 - PARTICLE_MODE_NEAREST_POINTS: hold 'f' to disable force"; 						
	}
	if( key == '4'){
		currentMode = PARTICLE_MODE_NOISE;
		currentModeStr = "4 - PARTICLE_MODE_NOISE: snow particle simulation"; 						
		resetParticles();
	}	
		
	if( key == ' ' ){
		resetParticles();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
