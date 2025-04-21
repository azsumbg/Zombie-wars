#pragma once

#ifdef BASESERV_EXPORTS
#define BASE_API __declspec(dllexport)
#else
#define BASE_API __declspec(dllimport)
#endif

#include <random>

constexpr float scr_width{ 1000.0f };
constexpr float scr_height{ 800.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 750.0f };
constexpr float up_ground_boundary{ 295.0f };

enum class dirs { up = 0, down = 1, left = 2, right = 3, stop = 4 };
enum class states { stand = 0, run = 1, fight = 2, punch = 3, kick1 = 4, kick2 = 5 };

constexpr unsigned char no_type{ 0 };
constexpr unsigned char hero{ 1 };
constexpr unsigned char zombie1{ 2 };
constexpr unsigned char zombie2{ 4 };
constexpr unsigned char zombie3{ 8 };
constexpr unsigned char zombie4{ 16 };
constexpr unsigned char soul{ 32 };

constexpr unsigned char tomb{ 1 };
constexpr unsigned char house1{ 2 };
constexpr unsigned char house2{ 4 };
constexpr unsigned char house3{ 8 };
constexpr unsigned char potion{ 16 };

struct BASE_API FPOINT
{
	float x{ 0 };
	float y{ 0 };
};

namespace dll
{
	class BASE_API RANDIT
	{
	private:
		std::random_device rd{};
		std::seed_seq* sq{ nullptr };
		std::mt19937* twister{ nullptr };

	public:
		RANDIT();
		~RANDIT();

		int operator()(int min, int max);
	};

	template<typename T> class BAG
	{
	private:
		T* m_ptr{ nullptr };
		size_t m_size{ 0 };
		size_t m_pos{ 0 };

		bool in_valid_state = false;

	public:
		
		BAG() :m_size{ 1 }, m_ptr{ reinterpret_cast<T*>(calloc(1,sizeof(T))) } {};
		BAG(size_t max_size) :m_size{ max_size }, m_ptr{ reinterpret_cast<T*>(calloc(max_size, sizeof(T))) } 
		{
			in_valid_state = true;
		};
		
		~BAG()
		{
			if (m_ptr)free(m_ptr);
		}

		bool is_valid() const
		{
			return in_valid_state;
		}

		void push_back(T& element)
		{
			if (m_ptr)
			{
				if (m_pos < m_size)
				{
					*(m_ptr + m_pos) = element;
					++m_pos;
				}
				else
				{
					T* temp_m_ptr = reinterpret_cast<T*>(calloc(m_size + 1, sizeof(T)));
					for (size_t count = 0; count < m_size; ++count)*(temp_m_ptr + count) = *(m_ptr + count);
					*(temp_m_ptr + m_size) = element;
					++m_size;
					++m_pos;
					free(m_ptr);
					m_ptr = temp_m_ptr;
				}
				if (!in_valid_state)in_valid_state = true;
			}
		}
		void push_back(T&& element)
		{
			if (m_ptr)
			{
				if (m_pos < m_size)
				{
					*(m_ptr + m_pos) = element;
					++m_pos;
				}
				else
				{
					T* temp_m_ptr = reinterpret_cast<T*>(calloc(m_size + 1, sizeof(T)));
					for (size_t count = 0; count < m_size; ++count)*(temp_m_ptr + count) = *(m_ptr + count);
					*(temp_m_ptr + m_size) = element;
					++m_size;
					++m_pos;
					free(m_ptr);
					m_ptr = temp_m_ptr;
				}
				if (!in_valid_state)in_valid_state = true;
			}
		}

		void push_front(T& element)
		{
			*m_ptr = element;
		}
		void push_front(T&& element)
		{
			*m_ptr = element;
		}
		
		T front() const
		{
			return *m_ptr;
		}
		T back() const
		{
			return *m_ptr[m_pos - 1];
		}

		size_t size() const
		{
			return m_size;
		}

		T operator[](size_t index)
		{
			T ret{};
			if (index >= m_pos || index < 0 || !in_valid_state)return ret;

			return *(m_ptr + index);
		}

		void operator()(size_t index, T& element)
		{
			if (index < 0 || index >= m_size)return;
			*(m_ptr + index) = element;
		}
	};

	class BASE_API PROTON
	{
	protected:
		float width{ 0 };
		float height{ 0 };

	public:
		FPOINT start{};
		FPOINT end{};
		FPOINT center{};

		float x_radius{ 0 };
		float y_radius{ 0 };

		PROTON(float _x, float _y, float _width, float _height);
		PROTON();
		virtual ~PROTON() {};

		void SetEdges();
		void NewDims(float _width, float _height);
		void SetWidth(float _width);
		void SetHeight(float _height);

		float GetWidth() const;
		float GetHeight() const;
	};

	class BASE_API BASE :public PROTON
	{
	protected:
		
		float move_sx = 0;
		float move_sy = 0;
		float move_ex = 0;
		float move_ey = 0;
		float slope = 0;
		float intercept = 0;

		int strenght{ 0 };
		int attack_chance{ 0 };

		bool hor_line{ false };
		bool vert_line{ false };

		float speed = 1.0f;

		int frame{ 0 };
		int max_frames{ 0 };
		int frame_delay{ 0 };

		states state = states::stand;

		RANDIT ChanceToHit{};

		void SetPath(float __to_x, float __to_y);

	public:

		unsigned char type = no_type;
		dirs dir = dirs::stop;
		bool in_battle = false;
		
		int lifes{ 0 };

		BASE(unsigned char _what_type, float _put_x, float _put_y);
		virtual ~BASE() {};

		int GetFrame();
		int Attack();
		void ChangeState(states _to_what);
		states GetState() const;

		virtual void NextMove(BAG<FPOINT> _targets, float gear) = 0;
		virtual bool Move(float _where_x, float _where_y, float gear) = 0;
		virtual void Release() = 0;
	};

	typedef BASE* Creature;

	class BASE_API HERO :public BASE
	{
	private:

		HERO(float _where_x, float _where_y);

	public:

		void NextMove(BAG<FPOINT> _targets, float gear) override;
		bool Move(float _where_x, float _where_y, float gear) override;
		virtual void Release() override;

		friend BASE_API Creature Factory(unsigned char which_creature, float first_x, float first_y);
	};

	class BASE_API EVILS :public BASE
	{
	private:

		EVILS(unsigned char _what, float _where_x, float _where_y);

	public:

		void NextMove(BAG<FPOINT> _targets, float gear) override;
		bool Move(float _where_x, float _where_y, float gear) override;
		virtual void Release() override;

		friend BASE_API Creature Factory(unsigned char which_creature, float first_x, float first_y);
	};

	// FUNCTIONS *************************************

	float BASE_API Distance(FPOINT my_point, FPOINT target);

	void BASE_API sort(BAG<FPOINT>& cont, FPOINT targ);

	//FACTORY ***************************************

	BASE_API Creature Factory(unsigned char which_creature, float first_x, float first_y);
}