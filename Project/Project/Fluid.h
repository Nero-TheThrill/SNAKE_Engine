#pragma once
#include <list>
#include <vector>
#include "MyMath.h"


// Fluid magic numbers
const float FluidTimestep = 0.005f;
const float FluidSmoothLen = 0.012f;
const float FluidSmoothLenSq = FluidSmoothLen * FluidSmoothLen;
const float FluidSmoothLenInverse = 1.0f / FluidSmoothLen;
const float FluidSmoothLenPower6 = powf(FluidSmoothLenSq, 3);
const float FluidStaticStiff = 3000.0f;
const float FluidRestDensity = 1000.0f;
const float FluidRestDensityPower3Inverse = powf(1.0f / FluidRestDensity, 3);
const float FluidWaterMass = 0.0002f;
const float FluidViscosity = 0.1f;
const float FluidStiff = 200.0f;
const float FluidInitialSpacing = 0.0045f;

/*****************************************************************************/

struct FluidNeighborRecord
{
	unsigned int p = 0; // Particle Index
	unsigned int n = 0; // Neighbor Index		
	float distsq = 0; // Distance Squared and Non-Squared (After Density Calc)
};

struct FluidGridOffset
{
	unsigned int offset = 0; // offset into gridindices
	unsigned int count = 0; // number of particles in cell
};

/*****************************************************************************/

struct Particle
{
	Vector2 pos;
	Vector2 vel;
	Vector2 acc;
	float density = 0;
	float pressure = 0;
};

class Fluid
{
public:
	/* Common Interface */
	Fluid();
	~Fluid();

	void Create(float w, float h);
	void Fill(float size);
	void Clear();
	void Update(float dt);

	/* Common Data */
	unsigned int* gridindices;
	Particle* particles;
	unsigned int particles_size;

	FluidGridOffset* gridoffsets;
	unsigned int neighbors_capacity;
	unsigned int num_neighbors;
	FluidNeighborRecord* neighbors;

	unsigned int Size() { return particles_size; }
	unsigned int Step() { return step; }
	void Pause(bool p) { paused = p; }
	void PauseOnStep(unsigned int p) { pause_step = p; }
	float Width() { return width; }
	float Height() { return height; }
	inline Particle* particle_at(std::size_t index);
	bool Paused(void) const { return paused; }
private:

	/* Simulation */
	void UpdateGrid();
	__forceinline void ExpandNeighbors();
	void BuildNeighbors(int cell1, int cell2);
	void BuildNeighbors(int cell);
	void GetNeighbors();
	void ComputeDensity();
	void SqrtDist();
	void ComputeForce();
	void Integrate(float dt);

private:
	/* Run State */
	unsigned int step;
	bool paused;
	unsigned int pause_step;

	/* World Size */
	float width;
	float height;
	int grid_w;
	int grid_h;

	/* Coefficients for kernel */
	float poly6_coef;
	float grad_spiky_coef;
	float lap_vis_coef;

	std::vector<Vector3> planes;
	std::vector<Vector2> planes_2d;
};
