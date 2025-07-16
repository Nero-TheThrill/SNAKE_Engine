#include "Fluid.h"
#include "MyMath.h"


Fluid::Fluid()
{
	step = 0;
	paused = false;
	pause_step = 0xFFFFFFFF;

	width = 0;
	height = 0;
	grid_w = 0;
	grid_h = 0;

	gridindices = NULL;
	gridoffsets = NULL;
	num_neighbors = 0;
	// If this value is too small, ExpandNeighbors will fix it
	neighbors_capacity = 263 * 1200;
	neighbors = new FluidNeighborRecord[neighbors_capacity];

	// Precompute kernel coefficients
	// See "Particle-Based Fluid Simulation for Interactive Applications"
	// "Poly 6" Kernel - Used for Density
	poly6_coef = 315.0f / (64.0f * MyMath::pi * FluidSmoothLenPower6 * powf(FluidSmoothLen, 3));
	// Gradient of the "Spikey" Kernel - Used for Pressure
	grad_spiky_coef = -45.0f / (MyMath::pi * FluidSmoothLenPower6);
	// Laplacian of the "Viscosity" Kernel - Used for Viscosity
	lap_vis_coef = FluidViscosity * -grad_spiky_coef;

	particles = nullptr;
	particles_size = 0;
}

Fluid::~Fluid()
{
	Clear();
	delete[] gridoffsets; gridoffsets = NULL;
	num_neighbors = 0;
	neighbors_capacity = 0;
	delete[] neighbors; neighbors = neighbors;
}

// Create the fluid simulation
// width/height is the simulation world maximum size
void Fluid::Create(float w, float h)
{
	width = w;
	height = h;
	grid_w = (int)(width / FluidSmoothLen);
	grid_h = (int)(height / FluidSmoothLen);

	delete[] gridoffsets;
	gridoffsets = new FluidGridOffset[grid_w * grid_h];

	planes.clear();
	planes.reserve(4);
	planes.push_back(Vector3(1, 0, 0));
	planes.push_back(Vector3(0, 1, 0));
	planes.push_back(Vector3(-1, 0, width));
	planes.push_back(Vector3(0, -1, height));

	planes_2d.clear();
	planes_2d.reserve(4);
	planes_2d.push_back(Vector2{ -FluidStaticStiff, 0 });
	planes_2d.push_back(Vector2{0, -FluidStaticStiff });
	planes_2d.push_back(Vector2{FluidStaticStiff, 0 });
	planes_2d.push_back(Vector2{0, FluidStaticStiff });
}

Particle* Fluid::particle_at(std::size_t index)
{
	return particles + index;
}

// Fill a region in the lower left with evenly spaced particles
void Fluid::Fill(float size)
{
	Clear();

	int w = (int)(size / FluidInitialSpacing);

	// Allocate
	gridindices = new unsigned int[w * w];
	particles = new Particle[w * w];
	particles_size = w * w;

	// Populate
	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < w; y++)
		{
			particle_at(y * w + x)->pos = Vector2{ x * FluidInitialSpacing, Height() - y * FluidInitialSpacing };
			particle_at(y * w + x)->vel = Vector2{ 0, 0 };
			particle_at(y * w + x)->acc = Vector2{ 0, 0 };
			gridindices[y * w + x] = 0;
		}
	}
}

// Remove all particles
void Fluid::Clear()
{
	step = 0;

	delete[] particles;

	particles = nullptr;
	particles_size = 0;

	delete[] gridindices; gridindices = NULL;
}

// Expand the Neighbors list if necessary
// This function is rarely called
__forceinline void Fluid::ExpandNeighbors()
{
	// Increase the size of the neighbors array because it is full
	neighbors_capacity *= 2;
	FluidNeighborRecord* new_neighbors = new FluidNeighborRecord[neighbors_capacity];
	memcpy(new_neighbors, neighbors, sizeof(FluidNeighborRecord) * num_neighbors);
	delete[] neighbors;
	neighbors = new_neighbors;
}

// Simulation Update
// Build the grid of neighbors
// Imagine an evenly space grid.  All of our neighbors will be
// in our cell and the 8 adjacent cells
void Fluid::UpdateGrid()
{
	// Cell size is the smoothing length

	// Clear the offsets
	for (int offset = 0; offset < (grid_w * grid_h); offset++)
	{
		gridoffsets[offset].count = 0;
	}

	// Count the number of particles in each cell
	for (unsigned int particle = 0; particle < particles_size; particle++)
	{
		// Find where this particle is in the grid
		int p_gx = MyMath::min(MyMath::max((int)(particle_at(particle)->pos.x * (1.0 / FluidSmoothLen)), 0), grid_w - 1);
		int p_gy = MyMath::min(MyMath::max((int)(particle_at(particle)->pos.y * (1.0 / FluidSmoothLen)), 0), grid_h - 1);
		int cell = p_gy * grid_w + p_gx;
		gridoffsets[cell].count++;
	}

	// Prefix sum all of the cells
	unsigned int sum = 0;
	for (int offset = 0; offset < (grid_w * grid_h); offset++)
	{
		gridoffsets[offset].offset = sum;
		sum += gridoffsets[offset].count;
		gridoffsets[offset].count = 0;
	}

	// Insert the particles into the grid
	for (unsigned int particle = 0; particle < particles_size; particle++)
	{
		// Find where this particle is in the grid
		int p_gx = MyMath::min(MyMath::max((int)(particle_at(particle)->pos.x * (1.0 / FluidSmoothLen)), 0), grid_w - 1);
		int p_gy = MyMath::min(MyMath::max((int)(particle_at(particle)->pos.y * (1.0 / FluidSmoothLen)), 0), grid_h - 1);
		int cell = p_gy * grid_w + p_gx;
		gridindices[gridoffsets[cell].offset + gridoffsets[cell].count] = particle;
		gridoffsets[cell].count++;
	}
}
void Fluid::BuildNeighbors(int cell1, int cell2)
{
	// Loop over ever particle in the neighboring cell
	unsigned int* start1 = gridindices + gridoffsets[cell1].offset;
	const unsigned int* end1 = start1 + gridoffsets[cell1].count;
	for (; start1 < end1; ++start1)
	{
		const unsigned int P = *start1;
		Vector2 pos_P = particle_at(P)->pos;
		unsigned int* start2 = gridindices + gridoffsets[cell2].offset;
		const unsigned int* end2 = start2 + gridoffsets[cell2].count;
		for (; start2 < end2; ++start2)
		{
			// Distance squared
			const unsigned int N = *start2;
			const Vector2 d = pos_P - particle_at(N)->pos;
			const float distsq = d.x * d.x + d.y * d.y;

			// Check that the particle is within the smoothing length
			if (distsq < FluidSmoothLenSq)
			{
				if (num_neighbors >= neighbors_capacity)
				{
					ExpandNeighbors();
				}
				// Record the ID of the two particles
				// And record the squared distance
				FluidNeighborRecord& record = neighbors[num_neighbors];
				record.p = P;
				record.n = N;
				record.distsq = distsq;
				num_neighbors++;
			}
		}
	}
}



void Fluid::BuildNeighbors(int cell)
{
	// Loop over ever particle in the neighboring cell
	unsigned int* start1 = gridindices + gridoffsets[cell].offset;
	const unsigned int* end1 = start1 + gridoffsets[cell].count - 1;
	const unsigned int* end2 = end1 + 1;
	for (; start1 < end1; ++start1)
	{
		const unsigned int P = *start1;
		Vector2 pos_P = particle_at(P)->pos;
		for (unsigned int* start2 = start1 + 1; start2 < end2; ++start2)
		{
			// Distance squared
			const unsigned int N = *start2;
			const Vector2 d = pos_P - particle_at(N)->pos;
			const float distsq = d.x * d.x + d.y * d.y;

			// Check that the particle is within the smoothing length
			if (distsq < FluidSmoothLenSq)
			{
				if (num_neighbors >= neighbors_capacity)
				{
					ExpandNeighbors();
				}
				// Record the ID of the two particles
				// And record the squared distance
				FluidNeighborRecord& record = neighbors[num_neighbors];
				record.p = P;
				record.n = N;
				record.distsq = distsq;
				num_neighbors++;
			}
		}
	}
}
// Simulation Update
// Build a list of neighbors (particles from adjacent grid locations) for every particle
void Fluid::GetNeighbors()
{
	num_neighbors = 0;

	static int dy[4] = { 0,  1, 1,  1 };
	static int dx[4] = { 1, -1, 0,  1 };

	const int grid_size = (grid_w * grid_h);
	for (int g = 0; g < grid_size; ++g)
	{
		BuildNeighbors(g);
	}

	for (int grid_y = 0; grid_y < grid_h - 1; ++grid_y)
	{
		for (int grid_x = 0; grid_x < grid_w - 1; ++grid_x)
		{
			int cell = grid_y * grid_w + grid_x;

			for (int i = 0; i < 4; ++i)
			{
				int c_x = grid_x + dx[i];
				int c_y = grid_y + dy[i];
				int n_cell = c_y * grid_w + c_x;
				BuildNeighbors(cell, n_cell);
			}
		}
	}

	for (int grid_x = 0; grid_x < grid_w - 1; ++grid_x)
	{
		int cell = (grid_h - 1) * grid_w + grid_x;
		BuildNeighbors(cell, cell + 1);
	}

	for (int grid_y = 0; grid_y < grid_h - 1; ++grid_y)
	{
		int cell = grid_y * grid_w + (grid_w - 1);
		BuildNeighbors(cell, cell + grid_w);
	}
}

// Simulation Update
// Compute the density for each particle based on its neighbors within the smoothing length
void Fluid::ComputeDensity()
{
	const float density_init = FluidSmoothLenPower6 * FluidWaterMass;
	for (unsigned int particle = 0; particle < particles_size; particle++)
	{
		// This is r = 0
		particle_at(particle)->density = density_init;
	}

	const float f = FluidSmoothLen * FluidSmoothLen;
	const float fluidsmoothlen_power2 = f * f;
	const float fluidsmoothlen_power3 = fluidsmoothlen_power2 * f;

	// foreach neighboring pair of particles
	for (unsigned int i = 0; i < num_neighbors; i++)
	{
		// distance squared
		const float r2 = neighbors[i].distsq;
		const float r4 = r2 * r2;

		// Density is based on proximity and mass
		// Density is:
		// M_n * W(h, r)
		// Where the smoothing kernel is:
		// The the "Poly6" kernel
		// float h2_r2 = FluidSmoothLen * FluidSmoothLen - r2;
		const float dens = fluidsmoothlen_power3 - 3 * fluidsmoothlen_power2 * r2 + 3 * f * r4 - r4 * r2;

		const float density = FluidWaterMass * dens;

		particle_at(neighbors[i].p)->density += density;
		particle_at(neighbors[i].n)->density += density;
	}

	// Approximate pressure as an ideal compressible gas
	// based on a spring eqation relating the rest density
	for (unsigned int particle = 0; particle < particles_size; ++particle)
	{
		particle_at(particle)->density *= poly6_coef;

		//particle_at(particle)->pressure = FluidStiff * max(powf(particle_at(particle)->density / FluidRestDensity, 3) - 1, 0);
		particle_at(particle)->pressure = FluidStiff * MyMath::max(powf(particle_at(particle)->density, 3.f) * FluidRestDensityPower3Inverse - 1, 0.0f);
	}
}

inline float regress_sqrt(float x) {
	constexpr float a = 1.7804048140093766e-002f;
	constexpr float b = 2.6384206490682671e+000f;
	constexpr float c = -2.5307850486945682e+000f;

	//return a * x * x + b * x + c;
	return -5.2768771456141397e+005f * x * x + 1.5304131740404884e+002f * x + 7.5745722755439699e-004f;
}

// Simulation Update
// Perform a batch of sqrts to turn distance squared into distance
void Fluid::SqrtDist()
{
	for (unsigned int i = 0; i < num_neighbors; i++)
	{
		neighbors[i].distsq = sqrt(neighbors[i].distsq);
	}
}

// Simulation Update
// Compute the forces based on the Navier-Stokes equations for laminer fluid flow
// Follows is lots more voodoo
void Fluid::ComputeForce()
{
	// foreach neighboring pair of particles
	for (unsigned int i = 0; i < num_neighbors; i++)
	{
		// Compute force due to pressure and viscosity
		const float h_r = FluidSmoothLen - neighbors[i].distsq;
		Vector2 diff = particle_at(neighbors[i].n)->pos - particle_at(neighbors[i].p)->pos;

		// Forces is dependant upon the average pressure and the inverse distance
		// Force due to pressure is:
		// 1/rho_p * 1/rho_n * Pavg * W(h, r)
		// Where the smoothing kernel is:
		// The gradient of the "Spikey" kernel
		Vector2 force = (0.5f * (particle_at(neighbors[i].p)->pressure + particle_at(neighbors[i].n)->pressure) * grad_spiky_coef * h_r / neighbors[i].distsq) * diff;

		// Viscosity is based on relative velocity
		// Viscosity is:
		// 1/rho_p * 1/rho_n * Vrel * mu * W(h, r)
		// Where the smoothing kernel is:
		// The laplacian of the "Viscosity" kernel
		force += (lap_vis_coef * (particle_at(neighbors[i].n)->vel - particle_at(neighbors[i].p)->vel));

		// Throw in the common (h-r) * 1/rho_p * 1/rho_n
		force *= h_r * 1.0f / (particle_at(neighbors[i].p)->density * particle_at(neighbors[i].n)->density);

		Vector2 f = FluidWaterMass * force;
		// Apply force - equal and opposite to both particles
		particle_at(neighbors[i].p)->acc += f;
		particle_at(neighbors[i].n)->acc -= f;
	}
}

// Simulation Update
// Integration
void Fluid::Integrate(float dt)
{
	Vector2 gravity = { 0, 1 };
	for (unsigned int particle = 0; particle < particles_size; ++particle)
	{
		// Walls
		for (auto it = planes.begin(); it != planes.end(); it++)
		{
			float dist = particle_at(particle)->pos.x * (*it).x + particle_at(particle)->pos.y * (*it).y + (*it).z;
			particle_at(particle)->acc += MyMath::min(dist, 0.0f) * -FluidStaticStiff * Vector2{ (*it).x, (*it).y };
		}

		// Acceleration
		particle_at(particle)->acc += gravity;

		// Integration - Euler-Cromer		
		particle_at(particle)->vel += dt * particle_at(particle)->acc;
		particle_at(particle)->pos += dt * particle_at(particle)->vel;
		particle_at(particle)->acc = Vector2{ 0, 0 };
	}
}

// Simulation Update
void Fluid::Update(float dt)
{
	// Pause runs the simulation standing still for profiling
	if (paused || step == pause_step) { dt = 0.0f; }
	else { step++; }

	// Create neighbor information
	UpdateGrid();
	GetNeighbors();

	// Calculate the forces for all of the particles
	ComputeDensity();
	SqrtDist();
	ComputeForce();

	// And integrate
	Integrate(dt);
}
