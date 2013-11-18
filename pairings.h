#ifndef PAIRINGS_H
#define PAIRINGS_H

#include <vector>
#include <assert.h>
#include <utility>
#include <stack>
#include <stdint.h>

// goes over all ways to pair up the intgers 0..n-1 or {0} of n == 0
class pairings {
public:
	pairings(int n) : n(n), remaining(-1), len_prev_stack(0) {
		assert(n%2 == 0);
		stack.push(0);
		if (n > 0) {
			paired.reserve(n/2);
			this->operator++();
		}
	}
	operator bool() const {
		return !stack.empty();
	}
	const std::vector< std::pair<int,int> >& operator*() {
		return paired;
	}
	void operator++();
private:
	int n;
	uint64_t remaining;
	int len_prev_stack;
	std::vector< std::pair<int,int> > paired;
	std::stack<int> stack;
};


void pairings::operator++() {
	if (n==0) {while (!stack.empty()) stack.pop(); return;}
	while (!stack.empty()) {
#ifndef NDEBUG
		{	// all items are either paired or remaining
			uint64_t items = remaining;
			for (int i=0; i<paired.size(); ++i) {
				assert(paired[i].first < n);
				assert(paired[i].second < n);
				items |= 1<<paired[i].first;
				items |= 1<<paired[i].second;
			}
			for (int i=0; i<n; ++i) assert(items&(1<<i));
		}
#endif
		int i = stack.top();
		// for (int o=0; o<paired.size(); o++) printf("(%d %d) ", paired[o].first, paired[o].second); printf("\n");
		bool pushing = len_prev_stack < stack.size();
		len_prev_stack = stack.size();
		if (pushing) {
			if (i == n) {
				assert(paired.size()*2 == n);
				stack.pop();
				return;
			} else {
				for (int j=0; j<n; ++j) if (i != j && remaining&(1<<j)) {
					assert(i<n);
					assert(j<n);
					assert(remaining&(1<<i));
					assert(remaining&(1<<j));
					remaining ^= 1<<i;
					remaining ^= 1<<j;
					paired.push_back(std::make_pair(i,j));
					while (i<n && !(remaining&(1<<i))) i++;
					stack.push(i);
					assert(remaining&(1<<i));
					break;
				}
			}
		} else {
			assert(i<n);
			assert(!paired.empty());
			assert(paired.back().first == i);
			int j = paired.back().second;
			paired.pop_back();
			assert(!(remaining&(1<<j)));
			remaining ^= 1<<j;
			bool loop_broke = false;
			for (j=j+1; j<n; ++j) if (i != j && remaining&(1<<j)) {
				assert(i<n);
				assert(j<n);
				assert(remaining&(1<<j));
				remaining ^= 1<<j;
				paired.push_back(std::make_pair(i,j));
				while (i<n && !(remaining&(1<<i))) i++;
				stack.push(i);
				assert(remaining&(1<<i));
				loop_broke = true;
				break;
			}
			if (!loop_broke) {
				assert(!(remaining&(1<<i)));
				remaining ^= 1<<i;
				stack.pop();
			}
		}
	}
}

#endif
