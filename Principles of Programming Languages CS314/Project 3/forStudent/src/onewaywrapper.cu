/*
 **********************************************
 *  CS314 Principles of Programming Languages *
 *  Fall 2020                                 *
 **********************************************
 */
#include "utils.hpp"
#include "gpuHeaders.cuh"
#include "extra.cu"
#include <iostream>

using namespace std;

#define threadsPerBlock 256

int one_way_handshake(GraphData graph, int *& matches, int numthreads, bool extra_credit)
{
	int num_thread_blocks = (numthreads + threadsPerBlock - 1) / threadsPerBlock;
	
	int numVertices = graph.numNodes;
	int numEdges = graph.numEdges;
	
	//Prepare various GPU arrays that we're going to need:
	int * strongNeighbor_gpu;//will hold strongest neighbor for each vertex
	int * matches_gpu;//will hold the output
	int * src_gpu;//holds the src nodes in edge list
	int * dst_gpu;//holds the dst nodes in edge list
	int * weight_gpu;//holds the edge weights in edge list	
	int * temp1_gpu;//a temporary array for data we don't need to keep for long
	int * temp2_gpu;//a temporary array for data we don't need to keep for long
	int * temp3_gpu;//a temporary array for data we don't need to keep for long
	int * temp4_gpu;//a temporary array for data we don't need to keep for long
	
	/** YOUR CODE GOES BELOW (allocate GPU memory, and copy from CPU to GPU as appropriate **/

	/** YOUR CODE GOES ABOVE **/
	
	
    //matching loop
    int iter;
    for (iter = 0; ; iter++) {
		
		if(extra_credit) {
			/** YOUR CODE GOES BELOW (extra credit) **/

			/** YOUR CODE GOES ABOVE (extra credit) **/
		} else {
			//Step 1: Get strongest neighbor for each vertex/node
			int * strongNeighbor_cpu = (int *) malloc(sizeof(int) * numVertices);
			int * strongNeighborWeight_cpu = (int *) malloc(sizeof(int) * numVertices);
			for(int x = 0; x < numVertices; x++) {
				strongNeighbor_cpu[x] = -1;
			}
			for(int x = 0; x < numEdges; x++) {
				int src = graph.src[x];
				int dst = graph.dst[x];
				int wgt = graph.weight[x];
				//std::cerr << src << "->" << dst << ": " << wgt << "\n";
				if(strongNeighbor_cpu[src] == -1 || strongNeighborWeight_cpu[src] < wgt) {
					strongNeighbor_cpu[src] = dst;
					strongNeighborWeight_cpu[src] = wgt;
				}
			}
			
			//move data from CPU to GPU, and free the CPU arrays
			cudaMemcpy(strongNeighbor_gpu, strongNeighbor_cpu, numVertices * sizeof(int), cudaMemcpyHostToDevice);
			free(strongNeighbor_cpu);
			free(strongNeighborWeight_cpu);
		}
		
		//Step 2: check for each vertex whether there's a handshake
		check_handshaking_gpu<<<num_thread_blocks, threadsPerBlock>>>(strongNeighbor_gpu, matches_gpu, numVertices);
		
		//Step 3: filter
		
		//Step 3a: decide which edges to keep (marked with a 1) versus filter (marked with a 0)
		int * keepEdges_gpu = temp1_gpu;
		temp1_gpu = NULL;
		markFilterEdges_gpu<<<num_thread_blocks, threadsPerBlock>>>(src_gpu, dst_gpu, matches_gpu, keepEdges_gpu, numEdges);
		
		
		//Step 3b: get new indices (in edge list for next iteration) of the edges we're going to keep
		int * newEdgeLocs_gpu = keepEdges_gpu;
		keepEdges_gpu = NULL;
		for(int distance = 0; distance <= numEdges; distance = max(1, distance * 2)) {
			exclusive_prefix_sum_gpu<<<num_thread_blocks, threadsPerBlock>>>(newEdgeLocs_gpu, temp2_gpu, distance, numEdges+1);
			swapArray((void**) &newEdgeLocs_gpu, (void**) &temp2_gpu);
		}
		
		//note: temp1 is still in use, until we're done with newEdgeLocs_gpu
		
		//Step 3c: check if we're done matching
		int lastLoc = 0;
		cudaMemcpy(&lastLoc, &(newEdgeLocs_gpu[numEdges]), sizeof(int), cudaMemcpyDeviceToHost);
		if(lastLoc < 2) {
			//termination: fewer than two nodes remain unmatched
			break;
		} else if(lastLoc == numEdges) {
			//termination: no additional matches are possible
			break;
		}
		
		//Step 3d: pack the src, dst, and weight arrays in accordance with new edge locations
		packGraph_gpu<<<num_thread_blocks, threadsPerBlock>>>(temp2_gpu, src_gpu, temp3_gpu, dst_gpu, temp4_gpu, weight_gpu, newEdgeLocs_gpu, numEdges);
		swapArray((void**) &temp2_gpu, (void**) &src_gpu);
		swapArray((void**) &temp3_gpu, (void**) &dst_gpu);
		swapArray((void**) &temp4_gpu, (void**) &weight_gpu);
		
		temp1_gpu = newEdgeLocs_gpu;
		newEdgeLocs_gpu = NULL;
		
		//note: now we're done with the current contents of all the temporary arrays
		
		//Set new number of edges:
		numEdges = lastLoc;
		
		if(iter > numVertices) {
			cerr << "Error: matching has been running too long; breaking loop now\n";
			break;
		}
		
		if(!extra_credit) {
			//Step 4: Copy new graph arrays to CPU
			cudaMemcpy(graph.src, src_gpu, numEdges * sizeof(int), cudaMemcpyDeviceToHost);
			cudaMemcpy(graph.dst, dst_gpu, numEdges * sizeof(int), cudaMemcpyDeviceToHost);
			cudaMemcpy(graph.weight, weight_gpu, numEdges * sizeof(int), cudaMemcpyDeviceToHost);
		}
    }
	
	cudaMemcpy(matches, matches_gpu, numVertices * sizeof(int), cudaMemcpyDeviceToHost);
	
	//Wait until pending GPU operations are complete:
	cudaDeviceSynchronize();
	
	//free GPU arrays
	/** YOUR CODE GOES BELOW **/

	/** YOUR CODE GOES ABOVE **/
	
	cudaError_t cudaError;
	cudaError = cudaGetLastError();
	if(cudaError != cudaSuccess) {
		cerr << "Warning: one or more CUDA errors occurred. Try using cuda-gdb to debug. Error message: \n\t" <<cudaGetErrorString(cudaError) << "\n";
	}
	
	return iter + 1;
}

void one_way_handshake_wrapper(GraphData graph, int *& matches, int numthreads, bool extra_credit)
{
	fprintf(stderr, "Start One Way Matching ... \n");

    struct timeval beginTime, endTime;

    setTime(&beginTime);

	int iter = one_way_handshake(graph, matches, numthreads, extra_credit);

    setTime(&endTime);

    fprintf(stderr, "Done matching.\n");

    fprintf(stderr, "Performed matching for %d iterations\n", iter);
    fprintf(stderr, "One Way Handshaking Matching Time: %.2f ms\n",
            getTime(&beginTime, &endTime));
}
