#include <tuple>
#include <meta/meta.hpp>

namespace fsm
{

namespace detail
{
struct EvEnterState {};
struct EvExitState {};

template<typename T>
struct state {
	T actual;

	template<typename E>
	void event(const E &event)
	{
		actual.event(event);
	}

	void event(const EvEnterState &)
	{
		actual.enter();
	}

	void event(const EvExitState &)
	{
		actual.exit();
	}
};

} // namespace detail

template<typename S1, typename E, typename S2>
struct transition_definition
{
	using start_t = detail::state<S1>;
	using stop_t = detail::state<S2>;
	using event_t = E;
};


struct transition_to_state {
	template<typename T>
	using invoke = typename T::start_t;
};

template<typename T>
using all_states = meta::transform<T, transition_to_state>;

template<typename T>
using all_unique_states = meta::unique<all_states<T>>;

template<typename Transitions>
struct fsm
{
	struct state_events {
		template<typename T>
		using invoke = meta::list<typename T::start_t, typename T::event_t>;
	};

	using all_state_events = meta::transform<Transitions, state_events>;
	using states_t = meta::apply<meta::quote<std::tuple>, all_unique_states<Transitions>>;

	template<typename S, typename E>
	using handle_event = meta::count<
		all_state_events,
		meta::list<S, E>>;

	template<typename S, typename E>
	struct check_dest {
		template<typename T>
		using invoke = meta::and_<std::is_same<S, typename T::start_t>, std::is_same<E, typename T::event_t>>;
	};
	
	template<typename S, typename E>
	using destination = meta::front<meta::find_if<Transitions, check_dest<S, E>>>;

	template<typename S, typename E>
	using destination_state = typename destination<S, E>::stop_t;

	template<typename I>
	using current_state = std::tuple_element_t<I::value, states_t>;

	template<typename T>
	using state_to_index = meta::find_index<meta::as_list<states_t>, T>;

	template<typename I, typename E>
	using next_state = destination_state<current_state<I>, E>;

	template<typename I, typename E>
	using next_state_index = state_to_index<next_state<I, E>>;

	fsm()
	{
		current = 0;
		std::get<0>(states).event(detail::EvEnterState{});
	}

	template<typename E>
	void on(const E &event)
	{
		onCheck(event, current, meta::make_index_sequence<state_count::value>{});
	}

	template<typename E, std::size_t... Is>
	void onCheck(const E &event, std::size_t atIdx, meta::index_sequence<Is...>)
	{
		int dummy[state_count::value] = {
			(
				onImpl<E, std::integral_constant<std::size_t, Is>>(event, atIdx),
				0
			)...
		};
	}

	template<typename E, typename I>
	std::enable_if_t<handle_event<std::tuple_element_t<I::value, states_t>, E>::value == 1>
	onImpl(const E &event, std::size_t atIdx, std::tuple_element_t<I::value, states_t>* = nullptr)
	{
		if (I::value == atIdx) {
			std::get<I::value>(states).event(event);
			std::get<I::value>(states).event(detail::EvExitState{});

			std::get<
				next_state_index<I, E>::value
			>(states).event(detail::EvEnterState{});

			current = next_state_index<I, E>::value;
		}
	}

	template<typename E, typename I>
	std::enable_if_t<handle_event<std::tuple_element_t<I::value, states_t>, E>::value == 0>
	onImpl(const E &event, std::size_t atIdx, std::tuple_element_t<I::value, states_t>* = nullptr)
	{
	}

private:
	states_t states;
	using state_count = std::tuple_size<states_t>;
	int current;
};

} // namespace fsm
