import os
import sys
import subprocess
import pydot

class Vcg2Dot(object):
    def __init__(self, vcg_file, dot_file):
        self.vcg_file = vcg_file
        self.dot_file = dot_file

    def vcg_to_dot(self):
        print("graph-easy --input=" + self.vcg_file + " -as=dot --output=" + self.dot_file)
        subprocess.run("graph-easy --input=" + self.vcg_file + " -as=dot --output=" + self.dot_file, shell=True)
        
class VcgFiles2Dot(object):
    def __init__(self, vcg_folder, dot_folder):
        self.vcg_folder = vcg_folder
        self.dot_folder = dot_folder

    def vcg_to_dot(self):
        if not os.path.exists(self.dot_folder):
            os.makedirs(self.dot_folder)
        
        for file in os.listdir(self.vcg_folder):
            vcg_to_dot = Vcg2Dot(self.vcg_folder + file, self.dot_folder + file + ".dot")
            vcg_to_dot.vcg_to_dot()
            
class CallgraphInfoCombiner(object):
    def __init__(self, dot_folder, function_name, output_file) -> None:
        self._dot_folder = dot_folder
        self._funciont_name = function_name
        self._output_file = output_file
        self._callee = dict()
        self._graph = pydot.Dot("callgraph-info-combiner", graph_type="digraph", bgcolor="white")
        pass
    
    def analyze(self, include_private=False):
        for file in os.listdir(self._dot_folder):
            self._read_dot(self._dot_folder + "/" + file)
            
        nodes_in_graph = set()
        if self._funciont_name in self._callee:
            self._add_node_and_edge(self._funciont_name, nodes_in_graph, include_private)
        self._graph.write_dot(self._output_file + ".dot")
        self._graph.write_png(self._output_file + ".png")
        
    def _add_node_and_edge(self, node_name, nodes_in_graph, include_private=False):
        if include_private and node_name.startswith('"') and node_name.endswith('"'):
            return
        
        if node_name not in nodes_in_graph:
            print("add node: " + node_name)
            self._graph.add_node(pydot.Node(node_name))
            nodes_in_graph.add(node_name)
            
        if node_name in self._callee:
            for callee in self._callee[node_name]:
                if include_private == False and callee.startswith('"') and callee.endswith('"'):
                    continue
        
                if callee not in nodes_in_graph:
                    self._add_node_and_edge(callee, nodes_in_graph, include_private)
                self._graph.add_edge(pydot.Edge(node_name, callee))
                print("add edge: " + node_name + " -> " + callee)
        
    
    def _read_dot(self, dot_file):
        graphs = pydot.graph_from_dot_file(dot_file)
        for graph in graphs:                    
            for edge in graph.get_edges():
                if edge.get_source() in self._callee:
                    self._callee[edge.get_source()].add(edge.get_destination())
                else:
                    self._callee[edge.get_source()] = {edge.get_destination()}
            
if __name__ == '__main__':
    vcg_folder = sys.argv[1]
    function_name = sys.argv[2]
    output_file = sys.argv[3]
    
    dot_folder = "/tmp/CallgraphInfoCombiner/dot/"
    
    vcg_to_dot = VcgFiles2Dot(vcg_folder, dot_folder)
    vcg_to_dot.vcg_to_dot()
    CallgraphInfoCombiner(dot_folder, function_name, output_file).analyze()