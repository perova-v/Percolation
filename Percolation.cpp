#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <random>
#include <algorithm>

struct DSU {
    std::vector<int> parent;

    DSU(size_t size) {
        parent.resize(size);
        for (size_t i = 0; i < size; ++i) {
            parent[i] = i;
        }
    }

    int find(int i) {
        if (parent[i] == i)
            return i;
        return parent[i] = find(parent[i]);
    }

    void unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j) {
            parent[root_i] = root_j;
        }
    }
};

class Percolation {
private:
    size_t n;
    std::vector<bool> opened;
    DSU dsu;
    size_t open_sites_count;

    int virtual_top;
    int virtual_bottom;

    int get_index(size_t row, size_t col) const {
        return row * n + col;
    }

public:
    Percolation(size_t dimension) 
        : n(dimension), 
          opened(dimension * dimension, false), 
          dsu(dimension * dimension + 2),
          open_sites_count(0) 
    {
        virtual_top = dimension * dimension;
        virtual_bottom = dimension * dimension + 1;
    }

    void open(size_t row, size_t col) {
        if (isOpen(row, col)) return;

        int idx = get_index(row, col);
        opened[idx] = true;
        open_sites_count++;

        if (row == 0) {
            dsu.unite(idx, virtual_top);
        }
        if (row == n - 1) {
            dsu.unite(idx, virtual_bottom);
        }

        if (row > 0 && isOpen(row - 1, col)) {
            dsu.unite(idx, get_index(row - 1, col));
        }
        if (row < n - 1 && isOpen(row + 1, col)) {
            dsu.unite(idx, get_index(row + 1, col));
        }
        if (col > 0 && isOpen(row, col - 1)) {
            dsu.unite(idx, get_index(row, col - 1));
        }
        if (col < n - 1 && isOpen(row, col + 1)) {
            dsu.unite(idx, get_index(row, col + 1));
        }
    }

    bool isOpen(size_t row, size_t col) const {
        return opened[get_index(row, col)];
    }

    bool percolates() {
        return dsu.find(virtual_top) == dsu.find(virtual_bottom);
    }

    size_t numberOfOpenSites() const {
        return open_sites_count;
    }
};

struct PercolationStats {
private:
    size_t n;
    size_t trials_count;
    double mean_val;
    double stddev_val;
    double conf_low;
    double conf_high;

public:
    PercolationStats(size_t dimension, size_t trials)
        : n(dimension), trials_count(trials), mean_val(0), stddev_val(0), conf_low(0), conf_high(0) {}

    double get_mean() const { return mean_val; }
    double get_standard_deviation() const { return stddev_val; }
    double get_confidence_low() const { return conf_low; }
    double get_confidence_high() const { return conf_high; }

    void execute() {
        if (trials_count == 0) return;

        std::vector<double> thresholds;
        thresholds.reserve(trials_count);

        std::random_device rd;
        std::mt19937 gen(rd());

        std::vector<std::pair<size_t, size_t>> cells;
        cells.reserve(n * n);
        for (size_t r = 0; r < n; ++r) {
            for (size_t c = 0; c < n; ++c) {
                cells.push_back({r, c});
            }
        }

        for (size_t t = 0; t < trials_count; ++t) {
            Percolation perc(n);
            
            std::shuffle(cells.begin(), cells.end(), gen);

            size_t cell_index = 0;
            while (!perc.percolates() && cell_index < cells.size()) {
                auto [r, c] = cells[cell_index];
                perc.open(r, c);
                cell_index++;
            }

            double threshold = static_cast<double>(perc.numberOfOpenSites()) / (n * n);
            thresholds.push_back(threshold);
        }

        double sum = std::accumulate(thresholds.begin(), thresholds.end(), 0.0);
        mean_val = sum / trials_count;
        
        if (trials_count > 1) {
            double sq_sum = 0.0;
            for (double val : thresholds) {
                sq_sum += (val - mean_val) * (val - mean_val);
            }
            stddev_val = std::sqrt(sq_sum / (trials_count - 1));
        } else {
            stddev_val = 0.0;
        }

        double margin = 1.96 * stddev_val / std::sqrt(trials_count);
        conf_low = mean_val - margin;
        conf_high = mean_val + margin;
    }
};

int main() {
    size_t n = 100;
    size_t trials = 100;

    PercolationStats stats(n, trials);
    stats.execute();

    std::cout << "Mean (p*): " << stats.get_mean() << std::endl;
    std::cout << "Standard deviation (s): " << stats.get_standard_deviation() << std::endl;
    std::cout << "95% confidence interval: [" 
              << stats.get_confidence_low() << ", " 
              << stats.get_confidence_high() << "]" << std::endl;

    return 0;
}