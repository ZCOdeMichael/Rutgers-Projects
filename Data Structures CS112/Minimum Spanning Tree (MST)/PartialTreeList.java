package app;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.NoSuchElementException;

import structures.Arc;
import structures.Graph;
import structures.PartialTree;
import structures.Vertex;

/**
 * Stores partial trees in a circular linked list
 * 
 */
public class PartialTreeList implements Iterable<PartialTree> {
    
	/**
	 * Inner class - to build the partial tree circular linked list 
	 * 
	 */
	public static class Node {
		/**
		 * Partial tree
		 */
		public PartialTree tree;
		
		/**
		 * Next node in linked list
		 */
		public Node next;
		
		/**
		 * Initializes this node by setting the tree part to the given tree,
		 * and setting next part to null
		 * 
		 * @param tree Partial tree
		 */
		public Node(PartialTree tree) {
			this.tree = tree;
			next = null;
		}
	}

	/**
	 * Pointer to last node of the circular linked list
	 */
	private Node rear; // SHOULD BE PRIVTE
	
	/**
	 * Number of nodes in the CLL
	 */
	private int size;
	
	/**
	 * Initializes this list to empty
	 */
    public PartialTreeList() {
    	rear = null;
    	size = 0;
    }

    /**
     * Adds a new tree to the end of the list
     * 
     * @param tree Tree to be added to the end of the list
     */
    public void append(PartialTree tree) {
    	Node ptr = new Node(tree);
    	if (rear == null) {
    		ptr.next = ptr;
    	} else {
    		ptr.next = rear.next;
    		rear.next = ptr;
    	}
    	rear = ptr;
    	size++;
    }

    /**
	 * Initializes the algorithm by building single-vertex partial trees
	 * 
	 * @param graph Graph for which the MST is to be found
	 * @return The initial partial tree list
	 */
	public static PartialTreeList initialize(Graph graph) {
	
		/* COMPLETE THIS METHOD */

		
		PartialTreeList result = new PartialTreeList(); // Creates a new PartialTreeList
		
		for(int i = 0; i < graph.vertices.length; i++) {
			
			PartialTree current = new PartialTree(graph.vertices[i]); // Creates PartialTree with a vertex in graph
						
			Vertex.Neighbor ptr = graph.vertices[i].neighbors; // Creates a pointer to traverse through all neighboring arcs
			while(ptr != null) {
				Arc arcCurrent = new Arc(graph.vertices[i], ptr.vertex, ptr.weight); // Creates an Arc for this current ptr

                current.getArcs().insert(arcCurrent); // inserts the arc in the Partial tree for the current vertex

                ptr = ptr.next; // Traverses to the next arc
			}
			
			result.append(current); // Adds Partial tree to the PartialTreeList
		}
		
		return result;
	}
	
	
	/**
	 * Executes the algorithm on a graph, starting with the initial partial tree list
	 * for that graph
	 * 
	 * @param ptlist Initial partial tree list
	 * @return Array list of all arcs that are in the MST - sequence of arcs is irrelevant
	 */
	
	public static ArrayList<Arc> execute(PartialTreeList ptlist) {
		
		/* COMPLETE THIS METHOD */
		ArrayList<Arc> result = new ArrayList<>();
		
		if(ptlist.size() == 0) {
			return result;
		}
		
		while(ptlist.size() != 1) {
			//System.out.println(ptlist.size());
			PartialTree PTX = ptlist.remove(); // Removes PTX from L
			structures.MinHeap<Arc> heap = PTX.getArcs(); // Get the Arcs from PTX
			Arc PQX = heap.deleteMin(); // Get PQX from PTX's Arcs
			
			while(PQX != null) {
				Vertex v1 = PQX.getv1(); // Gets first vertex
				Vertex v2 = PQX.getv2(); // Gets Second vertex
				
				//System.out.println(v1);
				//System.out.println(v2);
				
				
				if(v2.getRoot() == PTX.getRoot()) { // Check if v2 is belongs to PTX
					PQX = heap.deleteMin();
				}else { // If v2 does not belongs to PTX
					PartialTree temp = ptlist.removeTreeContaining(v2);
					//System.out.println("*****");
					//System.out.println(temp);
					//System.out.println(PTX);
					//System.out.println("*****");
					PTX.merge(temp);
					result.add(PQX);
					//System.out.println(PTX);
					ptlist.append(PTX);
					break;
				}
				
				
			}
			
			
		}
		
		
		return result;
	}
	
    /**
     * Removes the tree that is at the front of the list.
     * 
     * @return The tree that is removed from the front
     * @throws NoSuchElementException If the list is empty
     */
    public PartialTree remove() 
    throws NoSuchElementException {
    			
    	if (rear == null) {
    		throw new NoSuchElementException("list is empty");
    	}
    	PartialTree ret = rear.next.tree;
    	if (rear.next == rear) {
    		rear = null;
    	} else {
    		rear.next = rear.next.next;
    	}
    	size--;
    	return ret;
    		
    }

    /**
     * Removes the tree in this list that contains a given vertex.
     * 
     * @param vertex Vertex whose tree is to be removed
     * @return The tree that is removed
     * @throws NoSuchElementException If there is no matching tree
     */
    public PartialTree removeTreeContaining(Vertex vertex) 
    throws NoSuchElementException {
    	/* COMPLETE THIS METHOD */
    	if(rear == null) {
    		throw new NoSuchElementException("List is empty");
    	}
    	
    	PartialTree result = null;
    	Node ptr = rear, prev = rear;
    	
    	while(prev.next != ptr) {
    		prev = prev.next;
    	}
    	
    	
    	do {
    		PartialTree current = ptr.tree;
    		
    		
    		if(vertex.getRoot() == current.getRoot()) {
    			
    			result = current;
    			
    			if(ptr.next == ptr && prev == ptr) {
    				rear = null;
    				size--;
    				break;
    			}
    			
    			if(ptr == rear) {

    				prev = rear;
    				while(prev.next != rear) {
    					prev = prev.next;
    				}
    				rear = ptr.next;
    				prev.next = rear;
    			
    			}else if(ptr.next == rear) {
    				prev.next = rear;
    			}else {
    				prev.next = ptr.next;
    			}
    			size--;
    			break;
    		}
    		prev = ptr;
    		ptr = ptr.next;
    		
    		
    	}while(ptr != rear);
    	
    	
    	return result;
     }
    
    /**
     * Gives the number of trees in this list
     * 
     * @return Number of trees
     */
    public int size() {
    	return size;
    }
    
    /**
     * Returns an Iterator that can be used to step through the trees in this list.
     * The iterator does NOT support remove.
     * 
     * @return Iterator for this list
     */
    public Iterator<PartialTree> iterator() {
    	return new PartialTreeListIterator(this);
    }
    
    private class PartialTreeListIterator implements Iterator<PartialTree> {
    	
    	private PartialTreeList.Node ptr;
    	private int rest;
    	
    	public PartialTreeListIterator(PartialTreeList target) {
    		rest = target.size;
    		ptr = rest > 0 ? target.rear.next : null;
    	}
    	
    	public PartialTree next() 
    	throws NoSuchElementException {
    		if (rest <= 0) {
    			throw new NoSuchElementException();
    		}
    		PartialTree ret = ptr.tree;
    		ptr = ptr.next;
    		rest--;
    		return ret;
    	}
    	
    	public boolean hasNext() {
    		return rest != 0;
    	}
    	
    	public void remove() 
    	throws UnsupportedOperationException {
    		throw new UnsupportedOperationException();
    	}
    	
    }
}

