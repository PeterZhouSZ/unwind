#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/copyleft/marching_cubes.h>
#include <igl/components.h>
#include <igl/readOFF.h>
#include <igl/writeOFF.h>
#include <igl/decimate.h>
#include <igl/colormap.h>

#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <array>
#include <algorithm>

#include "MshLoader.h"

typedef igl::opengl::glfw::Viewer Viewer;


bool mesh_datfile(const std::string& dat_filename, Eigen::MatrixXd& V, Eigen::MatrixXi& F) {
  using namespace std;

  ifstream datfile(dat_filename);

  string raw_filename;
  datfile >> raw_filename;
  datfile >> raw_filename;
  raw_filename = string("./meshes/") + raw_filename;
  cout << "rawfile is " << raw_filename << endl;
  int w, h, d;
  string resolution_str;
  datfile >> resolution_str;
  datfile >> w;
  datfile >> h;
  datfile >> d;

  cout << "Grid has dimensions " << w << " x " << h << " x " << d << endl;

  char* data = new char[w*h*d];
  ifstream rawfile(raw_filename, std::ifstream::binary);
  rawfile.read(data, w*h*d);
  if (rawfile) {
    cout << "Read rawfile successfully" << endl;
  } else {
    cout << "Only read " << rawfile.gcount() << " bytes" << endl;
    return EXIT_FAILURE;
  }
  rawfile.close();

  Eigen::MatrixXd GP((w+2)*(h+2)*(d+2), 3);
  Eigen::VectorXd SV(GP.rows());

  int readcount = 0;
  int appendcount = 0;
  for (int zi = 0; zi < d+2; zi++) {
    for (int yi = 0; yi < h+2; yi++) {
      for (int xi = 0; xi < w+2; xi++) {
        if (xi == 0 || yi == 0 || zi == 0 || xi == (w+1) || yi == (h+1) || zi == (d+1)) {
          SV[readcount] = 0.0;
        } else {
          SV[readcount] = double(data[appendcount]);
          appendcount += 1;
        }
        GP.row(readcount) = Eigen::RowVector3d(xi, yi, zi);
        readcount += 1;
      }
    }
  }
  delete data;

  igl::copyleft::marching_cubes(SV, GP, w+2, h+2, d+2, V, F);

  igl::writeOFF("out.off", V, F);
}

void remove_garbage_components(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, Eigen::MatrixXi& newF) {
  using namespace std;

  cout << "Input model has " << V.rows() << " vertices and " << F.rows() << " faces" << endl;

  cout << "Computing connected components..." << endl;
  Eigen::VectorXi components;
  igl::components(F, components);
  vector<int> component_count;
  component_count.resize(components.maxCoeff());
  for (int i = 0; i < V.rows(); i++) {
    component_count[components[i]] += 1;
  }
  int max_component = -1;
  int max_component_count = 0;
  for (int i = 0; i < component_count.size(); i++) {
    if (max_component_count < component_count[i]) {
      max_component = i;
      max_component_count = component_count[i];
    }
  }

  cout << "The model has " << component_count.size() << " connected components." << endl;
  cout << "Component " << max_component << " has the most vertices with a count of " << max_component_count << endl;

  newF.resize(F.rows(), 3);

  int fcount = 0;
  for(int i = 0; i < F.rows(); i++) {
    bool keep = true;
    for (int j = 0; j < 3; j++) {
      if (components[F(i, j)] != max_component) {
        keep = false;
        break;
      }
    }
    if (keep) {
      newF.row(fcount++) = F.row(i);
    }
  }

  cout << "Output model has " << fcount << " faces and " << newF.maxCoeff() << " vertices." << endl;
  newF.conservativeResize(fcount, 3);
}

void decimate(const std::string& filename, int num_verts,
              Eigen::MatrixXd& Vdecimated, Eigen::MatrixXi& Fdecimated, Eigen::VectorXi& J) {
  using namespace std;
  Eigen::MatrixXd V;
  Eigen::MatrixXi F;
  igl::readOFF(filename, V, F);
  cout << "Decimating mesh " << Vdecimated << endl;
  igl::decimate(V, F, num_verts, Vdecimated, Fdecimated, J);
  cout << "Done!" << endl;

  igl::writeOFF("out3.off", Vdecimated, Fdecimated);
}

// Visualize the tet mesh as a wireframe
void visualize_tet_wireframe(igl::opengl::glfw::Viewer& viewer,
                             const Eigen::MatrixXd& TV,
                             const Eigen::MatrixXi& TT,
                             const Eigen::VectorXd& isovals)
{
  // Make a black line for each edge in the tet mesh which we'll draw
  std::vector<std::pair<int, int>> edges;
  for (int i = 0; i < TT.rows(); i++)
  {
    int tf1 = TT(i, 0);
    int tf2 = TT(i, 1);
    int tf3 = TT(i, 2);
    int tf4 = TT(i, 2);
    edges.push_back(std::make_pair(tf1, tf2));
    edges.push_back(std::make_pair(tf1, tf3));
    edges.push_back(std::make_pair(tf1, tf4));
    edges.push_back(std::make_pair(tf2, tf3));
    edges.push_back(std::make_pair(tf2, tf4));
    edges.push_back(std::make_pair(tf3, tf4));
  }

  Eigen::MatrixXd v1(edges.size(), 3), v2(edges.size(), 3);
  for (int i = 0; i < edges.size(); i++)
  {
    v1.row(i) = TV.row(edges[i].first);
    v2.row(i) = TV.row(edges[i].second);
  }

  // Normalize the isovalues between 0 and 1 for the colormap
  Eigen::MatrixXd C;
  const double isoval_min = isovals.minCoeff();
  const double isoval_max = isovals.maxCoeff();
  const double isoval_spread = isoval_max - isoval_min;
  const std::size_t n_isovals = isovals.size();
  Eigen::VectorXd isovals_normalized =
    (isovals - isoval_min * Eigen::VectorXd::Ones(n_isovals)) / isoval_spread;

  // Draw colored vertices of tet mesh based on their isovalue and black
  // lines connecting the vertices
  igl::colormap(igl::COLOR_MAP_TYPE_MAGMA, isovals_normalized, false, C);
  viewer.data().point_size = 5.0;
  viewer.data().add_points(TV, C);
  viewer.data().add_edges(v1, v2, Eigen::RowVector3d(0.1, 0.1, 0.1));
}


void load_yixin_tetmesh(const std::string& filename, Eigen::MatrixXd& TV, Eigen::MatrixXi& TF, Eigen::MatrixXi& TT) {
  using namespace std;
  MshLoader vol_loader(filename);
  assert(vol_loader.m_nodes_per_element == 4);
  assert(vol_loader.m_data_size == 8);

  int tv_rows = vol_loader.m_nodes.rows() / 3;
  int tt_rows = vol_loader.m_elements.rows() / vol_loader.m_nodes_per_element;

  TT.resize(tt_rows, vol_loader.m_nodes_per_element);
  TV.resize(tv_rows, 3);

  std::vector<array<int, 3>> tris;

  int vcount = 0;
  for (int i = 0; i < vol_loader.m_nodes.rows(); i += 3) {
    TV.row(vcount++) = Eigen::RowVector3d(vol_loader.m_nodes[i], vol_loader.m_nodes[i+2], vol_loader.m_nodes[i+1]);
  }
  int tcount = 0;
  for (int i = 0; i < vol_loader.m_elements.rows(); i += vol_loader.m_nodes_per_element) {
    const int e1 = vol_loader.m_elements[i];
    const int e2 = vol_loader.m_elements[i+1];
    const int e3 = vol_loader.m_elements[i+2];
    const int e4 = vol_loader.m_elements[i+3];
    TT.row(tcount++) = Eigen::RowVector4i(e1, e2, e3, e4);
    array<int, 3> t1, t2, t3, t4;
    t1 = array<int, 3>{{ e1, e2, e3 }};
    t2 = array<int, 3>{{ e1, e2, e3 }};
    t3 = array<int, 3>{{ e2, e3, e4 }};
    t4 = array<int, 3>{{ e1, e3, e4 }};
    sort(t1.begin(), t1.end());
    sort(t2.begin(), t2.end());
    sort(t3.begin(), t3.end());
    sort(t4.begin(), t4.end());
    tris.push_back(t1);
    tris.push_back(t2);
    tris.push_back(t3);
    tris.push_back(t4);
  }

  int fcount;
  TF.resize(tris.size(), 3);
  sort(tris.begin(), tris.end());
  for (int i = 0; i < TF.rows();) {
    int v1 = tris[i][0], v2 = tris[i][1], v3 = tris[i][2];
    int count = 0;
    while (v1 == tris[i][0] && v2 == tris[i][1] && v3 == tris[i][2]) {
      i += 1;
      count += 1;
    }
    if (count == 1) {
      TF.row(fcount++) = Eigen::RowVector3i(v1, v2, v3);
    }
  }

  TF.conservativeResize(fcount, 3);
}

int main(int argc, char *argv[]) {
  using namespace Eigen;
  using namespace std;
  igl::opengl::glfw::Viewer viewer;
  igl::opengl::glfw::imgui::ImGuiMenu menu;
  viewer.plugins.push_back(&menu);

  Eigen::MatrixXd TV;
  Eigen::MatrixXi TT, TF;
  Eigen::VectorXd isovals;
  load_yixin_tetmesh("outReoriented_.msh", TV, TF, TT);

  isovals.resize(TV.rows());
  for (int i = 0; i < TV.rows(); i++) {
    isovals[i] = TV.row(i).norm();
  }

  viewer.data().set_mesh(TV, TF);
  visualize_tet_wireframe(viewer, TV, TT, isovals);

  return viewer.launch();
}
