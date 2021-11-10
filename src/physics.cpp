#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

int type = 1;
int style = 0;
float elasticity = 0.5f;
float friction = 0.75f;
int object = 0;
float posA[3] = { 0.0f,1.0f,0.0f };
float posB[3] = { -3.0f,2.0f,-2.0f };
float posC[3] = { -4.0f,2.0f,2.0f };

float posParticles[3] = { 0.0f,9.0f,0.0f };

float posParticlesA[3] = { 1.0f,9.0f,0.0f };
float posParticlesB[3] = { -1.0f,9.0f,0.0f };

float radiusSphere = 1.0f;
float radiusCapsule = 1.0f;

float forceX = 0.0f;
float forceY = -9.8f;
float forceZ = 0.0f;

//Normales de los planos del eje X
float normalXRight[3] = { 0.f };
float normalXLeft[3] = { 0.f };

//Normales de los planos del eje Y
float normalYDown[3] = { 0.f };
float normalYTop[3] = { 0.f };

//Normales de los planos del eje Z
float normalZFront[3] = { 0.f };
float normalZBack[3] = { 0.f };

float dDown, dTop, dRight, dLeft, dFront, dBack;

struct particle{

	float posX, posY, posZ;
	float prePosX, prePosY, prePosZ;
	float postPosX, postPosY, postPosZ;
	float velX, velY, velZ;
	float postVelX, postVelY, postVelZ;
	float mass;
	float life;
	float tmpX, tmpY, tmpZ;

};

particle *particles = new particle[500];

namespace LilSpheres {
	extern const int maxParticles;
	extern void setupParticles(int numTotalParticles, float radius = 0.05f);
	extern void cleanupParticles();
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void drawParticles(int startIdx, int count);
}

bool show_test_window = false;
void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	{	//Propiedades
		ImGui::Begin("Propiedades");
		
		ImGui::Text("Seleccionar tipo:");
		//static int type = 0;
		ImGui::RadioButton("Fuente", &type, 1); ImGui::SameLine();
		
		if (type == 0) {
			ImGui::DragFloat3("Particles Pos.A", posParticlesA, 0.1f);
			ImGui::DragFloat3("Particles Pos.B", posParticlesB, 0.1f);
		}
		else {
			ImGui::DragFloat3("Particles Pos.", posParticles, 0.1f);
		}

		ImGui::Text("\nSeleccionar forma de calculo:");
		//static int style = 0;
		ImGui::RadioButton("Euler", &style, 0); ImGui::SameLine();
		
		ImGui::End();
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void NormalPlane(float* pointA, float* pointB, float* pointC, float* normal) {

	float vectorA[3] = { pointA[0] - pointB[0], pointA[1] - pointB[1], pointA[2] - pointB[2] };
	float vectorB[3] = { pointC[0] - pointB[0], pointC[1] - pointB[1], pointC[2] - pointB[2] };
	
	normal[0] = vectorA[1] * vectorB[2] - vectorA[2] * vectorB[1];
	normal[1] = vectorA[2] * vectorB[0] - vectorA[0] * vectorB[2];
	normal[2] = vectorA[0] * vectorB[1] - vectorA[1] * vectorB[0];

	//Normalizar el vector
	float modulo = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= modulo;
	normal[1] /= modulo;
	normal[2] /= modulo;

	//return normal;
}

void PhysicsInit() {
	//TODO
	srand(time(NULL));

	//Calcular la normal de los planos
	//Plano bajo
	float pointA[3] = { -5.0f, 0.0f, -5.0f };
	float pointB[3] = { -5.0f, 0.0f, 5.0f };
	float pointC[3] = { 5.0f, 0.0f, 5.0f };
	NormalPlane(pointA, pointB, pointC, normalYDown);
	dDown = -(normalYDown[0] * pointA[0]) - (normalYDown[1] * pointA[1]) - (normalYDown[2] * pointA[2]);

	for (int i = 0; i < 500; i++) {
		particles[i].posX = posParticles[0];
		particles[i].posY = posParticles[1];
		particles[i].posZ = posParticles[2];

		particles[i].prePosX = posParticles[0];
		particles[i].prePosY = posParticles[1];
		particles[i].prePosZ = posParticles[2];

		particles[i].velX = ((float)rand() / RAND_MAX) * 2 - 1;
		particles[i].velY = ((float)rand() / RAND_MAX) * 2 - 1;
		particles[i].velZ = ((float)rand() / RAND_MAX) * 2 - 1;

		particles[i].mass = 1.0f;

		particles[i].life = ((float)rand() / RAND_MAX) * 5 - 1;

	}
}

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

void PhysicsUpdate(float dt) {
	//TODO
	for (int i = 0; i < 500; ++i) {

		if (particles[i].life <= 0.0f) {

			if (type == 0) {
				particles[i].posX = RandomFloat(posParticlesA[0], posParticlesB[0]);
				particles[i].posY = RandomFloat(posParticlesA[1], posParticlesB[1]);
				particles[i].posZ = RandomFloat(posParticlesA[2], posParticlesB[2]);
			}
			else {
				particles[i].posX = posParticles[0];
				particles[i].posY = posParticles[1];
				particles[i].posZ = posParticles[2];
			}

			particles[i].prePosX = RandomFloat(particles[i].posX - 0.05f, particles[i].posX + 0.05f);
			particles[i].prePosY = particles[i].posY + 0.05f;
			particles[i].prePosZ = RandomFloat(particles[i].posZ - 0.05f, particles[i].posZ + 0.05f);

			particles[i].velX = ((float)rand() / RAND_MAX) * 2 - 1;
			particles[i].velY = ((float)rand() / RAND_MAX) * 2 - 1;
			particles[i].velZ = ((float)rand() / RAND_MAX) * 2 - 1;

			particles[i].life = ((float)rand() / RAND_MAX) * 5 - 1;

		}

		particles[i].life -= dt * 0.5f;


		//EULER
		// posicion
		particles[i].postPosX = particles[i].posX + dt * particles[i].velX;
		particles[i].postPosY = particles[i].posY + dt * particles[i].velY;
		particles[i].postPosZ = particles[i].posZ + dt * particles[i].velZ;

		//velocidad
		particles[i].postVelX = particles[i].velX + dt * (forceX / particles[i].mass);
		particles[i].postVelY = particles[i].velY + dt * (forceY / particles[i].mass);
		particles[i].postVelZ = particles[i].velZ + dt * (forceZ / particles[i].mass);

		
		

		float dotProductDown = (normalYDown[0] * particles[i].posX + normalYDown[1] * particles[i].posY + normalYDown[2] * particles[i].posZ);
		float dotProductPostDown = (normalYDown[0] * particles[i].postPosX + normalYDown[1] * particles[i].postPosY + normalYDown[2] * particles[i].postPosZ);

		//Detectar Colisiones
		//Plano de tierra
		if ((dotProductDown + dDown) * (dotProductPostDown + dDown) <= 0) {


				float dotProductPostVelDown = (normalYDown[0] * particles[i].postVelX + normalYDown[1] * particles[i].postVelY + normalYDown[2] * particles[i].postVelZ);
				float dotProductVelDown = (normalYDown[0] * particles[i].velX + normalYDown[1] * particles[i].velY + normalYDown[2] * particles[i].velZ);
				float normalVel[3] = { dotProductVelDown * normalYDown[0], dotProductVelDown * normalYDown[1], dotProductVelDown * normalYDown[2] };
				float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

				particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostDown + dDown)*normalYDown[0];
				particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostDown + dDown)*normalYDown[1];
				particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostDown + dDown)*normalYDown[2];

				particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelDown)*normalYDown[0];
				particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelDown)*normalYDown[1];
				particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelDown)*normalYDown[2];

				particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
				particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
				particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];

			 
		}


		//Colisiones Esfera
		float dist = sqrt(pow(particles[i].postPosX - posA[0], 2) + pow(particles[i].postPosY - posA[1], 2) + pow(particles[i].postPosZ - posA[2], 2));
		if (dist <= radiusSphere) {

			//Calcular la colision

			float P[3] = { particles[i].posX, particles[i].posY, particles[i].posZ };
			float Q[3] = { particles[i].postPosX, particles[i].postPosY, particles[i].postPosZ };
			float V[3] = { Q[0] - P[0], Q[1] - P[1], Q[2] - P[2] };

			float a = (pow(V[0], 2) + pow(V[1], 2) + pow(V[2], 2));
			float b = (2 * P[0] * V[0] - 2 * V[0] * posA[0] + 2 * P[1] * V[1] - 2 * V[1] * posA[1] + 2 * P[2] * V[2] - 2 * V[2] * posA[2]);
			float c = (pow(P[0], 2) - 2*P[0]*posA[0]+ pow(posA[0], 2) + pow(P[1], 2) - 2 * P[1] * posA[1] + pow(posA[1], 2) + pow(P[2], 2) - 2 * P[2] * posA[2] + pow(posA[2], 2)) - pow(radiusSphere, 2);

			float alpha[2] = { (-b + sqrt(pow(b,2) - 4 * a*c)) / (2 * a),
								(-b - sqrt(pow(b,2) - 4 * a*c)) / (2 * a),
			};

			float colision1[3] = { P[0] + V[0] * alpha[0], P[1] + V[1] * alpha[0] , P[2] + V[2] * alpha[0] };
			float colision2[3] = { P[0] + V[0] * alpha[1], P[1] + V[1] * alpha[1] , P[2] + V[2] * alpha[1] };

			float distCol1 = sqrt(pow(P[0] - colision1[0], 2) + pow(P[1] - colision1[1], 2) + pow(P[2] - colision1[2], 2));
			float distCol2 = sqrt(pow(P[0] - colision2[0], 2) + pow(P[1] - colision2[1], 2) + pow(P[2] - colision2[2], 2));

			float puntoColision[3] = { colision1[0], colision1[1], colision1[2] };

			if (distCol1 > distCol2) {
				puntoColision[0] = colision2[0];
				puntoColision[1] = colision2[1];
				puntoColision[2] = colision2[2];
			}
		

			
			float normalColision[3] = { puntoColision[0] - posA[0], puntoColision[1] - posA[1], puntoColision[2] - posA[2] };

			float dColision = -(normalColision[0] * puntoColision[0] + normalColision[1] * puntoColision[1] + normalColision[2] * puntoColision[2]);

			float dotProductColision = (normalColision[0] * particles[i].posX + normalColision[1] * particles[i].posY + normalColision[2] * particles[i].posZ);
			float dotProductPostColision = (normalColision[0] * particles[i].postPosX + normalColision[1] * particles[i].postPosY + normalColision[2] * particles[i].postPosZ);
			


			float dotProductPostVelColision = (normalColision[0] * particles[i].postVelX + normalColision[1] * particles[i].postVelY + normalColision[2] * particles[i].postVelZ);
			float dotProductVelColision = (normalColision[0] * particles[i].velX + normalColision[1] * particles[i].velY + normalColision[2] * particles[i].velZ);
			float normalVel[3] = { dotProductVelColision * normalColision[0], dotProductVelColision * normalColision[1], dotProductVelColision * normalColision[2] };
			float tangVel[3] = { particles[i].velX - normalVel[0], particles[i].velY - normalVel[1], particles[i].velZ - normalVel[2] };

			particles[i].postPosX = particles[i].postPosX - (1 + elasticity) * (dotProductPostColision + dColision)*normalColision[0];
			particles[i].postPosY = particles[i].postPosY - (1 + elasticity) * (dotProductPostColision + dColision)*normalColision[1];
			particles[i].postPosZ = particles[i].postPosZ - (1 + elasticity) * (dotProductPostColision + dColision)*normalColision[2];

			particles[i].postVelX = particles[i].postVelX - (1 + elasticity) * (dotProductPostVelColision)*normalColision[0];
			particles[i].postVelY = particles[i].postVelY - (1 + elasticity) * (dotProductPostVelColision)*normalColision[1];
			particles[i].postVelZ = particles[i].postVelZ - (1 + elasticity) * (dotProductPostVelColision)*normalColision[2];

			particles[i].postVelX = particles[i].postVelX - friction * tangVel[0];
			particles[i].postVelY = particles[i].postVelY - friction * tangVel[1];
			particles[i].postVelZ = particles[i].postVelZ - friction * tangVel[2];
			

		}

		particles[i].prePosX = particles[i].posX;
		particles[i].prePosY = particles[i].posY;
		particles[i].prePosZ = particles[i].posZ;

		particles[i].posX = particles[i].postPosX;
		particles[i].posY = particles[i].postPosY;
		particles[i].posZ = particles[i].postPosZ;

		particles[i].velX = particles[i].postVelX;
		particles[i].velY = particles[i].postVelY;
		particles[i].velZ = particles[i].postVelZ;

	}

	float *partVerts = new float[500 * 3];
	for (int i = 0; i < 500; ++i) {
		partVerts[i * 3 + 0] = particles[i].posX;
		partVerts[i * 3 + 1] = particles[i].posY;
		partVerts[i * 3 + 2] = particles[i].posZ;
	}
	LilSpheres::updateParticles(0, 500, partVerts);
	delete[] partVerts;

}
void PhysicsCleanup() {
	//TODO
}