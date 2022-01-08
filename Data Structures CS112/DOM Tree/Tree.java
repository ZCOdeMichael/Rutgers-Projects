package structures;

import java.util.*;

/**
 * This class implements an HTML DOM Tree. Each node of the tree is a TagNode, with fields for
 * tag/text, first child and sibling.
 * 
 */
public class Tree {
	
	/**
	 * Root node
	 */
	TagNode root=null;
	
	/**
	 * Scanner used to read input HTML file when building the tree
	 */
	Scanner sc;
	
	/**
	 * Initializes this tree object with scanner for input HTML file
	 * 
	 * @param sc Scanner for input HTML file
	 */
	public Tree(Scanner sc) {
		this.sc = sc;
		root = null;
	}
	
	/**
	 * Builds the DOM tree from input HTML file, through scanner passed
	 * in to the constructor and stored in the sc field of this object. 
	 * 
	 * The root of the tree that is built is referenced by the root field of this object.
	 */
	public void build() {
		/** COMPLETE THIS METHOD **/
		//System.out.println("Bulding..");
		Stack<TagNode> tracker = new Stack<TagNode>();
		do {
			String current = sc.nextLine();
			
			if(current == null) {
				continue;
			}else if(root == null) {
				root = new TagNode(getTag(current), null, null);
				tracker.push(root);
			}else {
				
				if(current.indexOf("<") != -1 && current.indexOf("</") == -1) { //<>
					if(tracker.peek().firstChild == null) {
						TagNode temp = new TagNode(getTag(current), null, null);
						tracker.peek().firstChild = temp;
						tracker.push(temp);
					}else {
						TagNode temp = new TagNode(getTag(current), null, null);
						getLastSib(tracker.peek().firstChild).sibling = temp;
						tracker.push(temp);
					}
					
				}else if(current.indexOf("<") != -1 && current.indexOf("</") != -1){//</>
					tracker.pop();
			
				}else {//Text
					if(tracker.peek().firstChild == null) {
						tracker.peek().firstChild = new TagNode(current, null, null);
					}else {
						getLastSib(tracker.peek().firstChild).sibling = new TagNode(current, null, null);
					}
				}
				
			}

			
		}while(sc.hasNext());
	}
	
	
	private TagNode getLastSib(TagNode tag) {
		for(TagNode ptr = tag; ptr != null; ptr = ptr.sibling) {
			tag = ptr;
		}
	
		return tag;
	}
	
	private String getTag(String tag) {
		return tag.substring(1, tag.length()-1);
	}
	
	/**
	 * Replaces all occurrences of an old tag in the DOM tree with a new tag
	 * 
	 * @param oldTag Old tag
	 * @param newTag Replacement tag
	 */
	public void replaceTag(String oldTag, String newTag) {//Am I allowed to do this?????
		/** COMPLETE THIS METHOD **/
		
		replace(root, oldTag, newTag);
	}
	
	private void replace(TagNode mainT, String oldT, String newT) { // CHECK INPUTS
		
		
		if(mainT == null) {
			return;
		}else {
			
			if(mainT.tag.equals(oldT)) {
				mainT.tag = newT;
			}
			

			replace(mainT.firstChild, oldT, newT);
			replace(mainT.sibling, oldT, newT);
		}
		
	}
	
	/**
	 * Boldfaces every column of the given row of the table in the DOM tree. The boldface (b)
	 * tag appears directly under the td tag of every column of this row.
	 * 
	 * @param row Row to bold, first row is numbered 1 (not 0).
	 */
	public void boldRow(int row) { // Only one table????
		/** COMPLETE THIS METHOD **/
		TagNode tempTable = getTable(root);
		TagNode tempRow = tempTable.firstChild;
		for(int i = 1; i < row; i++) {
			tempRow = tempRow.sibling;
		}
		boldRow(tempRow);
		
	}
	
	private void boldRow(TagNode row) {
		if(row.firstChild == null) {
			return;
		}
		for(TagNode td = row.firstChild; td != null; td = td.sibling) {
				TagNode temp = td.firstChild;
				td.firstChild = new TagNode("b", temp, null);
		}
	}
	
	private TagNode getTable(TagNode mainT) {
		
		if(mainT == null) {
			return null;
		}
		
		TagNode result = null;
		
		if(mainT.tag.equals("table")) {
			return mainT;
		}
		
		if(result == null) {
			result = getTable(mainT.firstChild);
		}
		if(result == null) {
			result = getTable(mainT.sibling);
		}
		return result;
		
		
	}
	
	/**
	 * Remove all occurrences of a tag from the DOM tree. If the tag is p, em, or b, all occurrences of the tag
	 * are removed. If the tag is ol or ul, then All occurrences of such a tag are removed from the tree, and, 
	 * in addition, all the li tags immediately under the removed tag are converted to p tags. 
	 * 
	 * @param tag Tag to be removed, can be p, em, b, ol, or ul
	 */
	public void removeTag(String tag) {
		/** COMPLETE THIS METHOD **/
	
		if(!(tag.equals("p") || tag.equals("em") || tag.equals("b") || tag.equals("ol") || tag.equals("ul"))) {
			return;
		}
		
		if(tag.equals("p") || tag.equals("em") || tag.equals("b")) {
			pRemove(tag, root, null);
		}else if(tag.equals("ol") || tag.equals("ul")){
			liRemove(root, tag);
			olRemove(tag, root, null); // ******* WIP ******* 
		}
		
	}
	
	private void liRemove(TagNode root, String tag) { // Replaces all the li's under the tag with p
		if(root == null) {
			return;
		}else {
			
			if(root.tag.equals(tag)) {
				for(TagNode ptr = root.firstChild; ptr != null; ptr = ptr.sibling) {
					
					if(ptr.tag.equals("li")) {
						ptr.tag = "p";
					}
				}
				
			}
			
			liRemove(root.firstChild, tag);
			liRemove(root.sibling, tag);
		
		}
		
		
		
		
	}
	
	private void olRemove(String tag, TagNode root, TagNode prev) { // ******* WIP ******* 
		if(root == null) {
			return;
		}else{ // REVISE THIS CODE!!!!!!!!!!! NOT FINISHED
			
			TagNode tempChild = root.firstChild;
			TagNode tempSibling = root.sibling;
			
			if(root.tag.equals(tag)) { // The current node's tag is equal to the tag
				
				if(prev.firstChild == root) { // The current node is the firstChild of the previous node
					
					if(root.firstChild != null && root.sibling == null) { // The current node has a firstChild but not a sibling
						prev.firstChild = root.firstChild;
						root.firstChild = null;
						
					}else if(root.firstChild != null && root.sibling != null){ // The current node has a firstChild and a sibling
						
						prev.firstChild = root.firstChild;
						getLastSibling(root.firstChild).sibling = root.sibling;
						root.firstChild = null;
						root.sibling = null;
						
					}else if(root.firstChild == null && root.sibling != null){ // The current node does not have a firstChild but has a sibling
						prev.firstChild = root.sibling;
						root.sibling = null;
						
					}else { // The current node does not have a firstChild and does not have a sibling
						prev.firstChild = null;
					}
					
				}else { // The current node is the sibling of the previous node
					
					if(root.firstChild != null && root.sibling == null) { // The current node has a firstChild but not a sibling
						prev.sibling = root.firstChild;
						root.firstChild = null;
					}else if(root.firstChild != null && root.sibling != null){ // The current node has a firstChild and a sibling 
						prev.sibling = root.firstChild;
						getLastSibling(root.firstChild).sibling = root.sibling;
						root.firstChild = null;
						root.sibling = null;
					}else if(root.firstChild == null && root.sibling != null){ // The current node does not have a firstChild but has a sibling
						prev.sibling = root.sibling;
						root.sibling = null;
					}else { // The current node does not have a firstChild and does not have a sibling
						prev.sibling = null;
					}
					
				}
				
				
			}
			
			
			olRemove(tag, tempChild, root);
			olRemove(tag, tempSibling, root);
		}
	}
	
	private void pRemove(String tag, TagNode root, TagNode prev) {
		
		
		if(root == null) {
			return;
		}else {
			
			TagNode tempChild = root.firstChild;
			TagNode tempSibling = root.sibling;
			
			if(root.tag.equals(tag)) { // The current node's tag is equal to the tag
				if(prev.firstChild == root) { // The current node is the firstChild of the previous node
					
					if(root.firstChild != null && root.sibling == null) { // The current node has a firstChild but not a sibling
						prev.firstChild = root.firstChild;
						root.firstChild = null;
						
					}else if(root.firstChild != null && root.sibling != null){ // The current node has a firstChild and a sibling
						getLastSibling(root.firstChild).sibling = root.sibling;
						prev.firstChild = root.firstChild;
						root.sibling = null;
						root.firstChild = null;
						
					}else if(root.firstChild == null && root.sibling != null){ // The current node does not have a firstChild but has a sibling
						prev.firstChild = root.sibling;
						root.sibling = null;
						
					}else { // The current node does not have a firstChild and does not have a sibling
						prev.firstChild = null;
					}
					
				}else { // The current node is the sibling of the previous node
					
					if(root.firstChild != null && root.sibling == null) { // The current node has a firstChild but not a sibling
						prev.sibling = root.firstChild;
						root.firstChild = null;
						
					}else if(root.firstChild != null && root.sibling != null){ // The current node has a firstChild and a sibling
						getLastSibling(root.firstChild).sibling = root.sibling;
						prev.sibling = root.firstChild;
						root.sibling = null;
						root.firstChild = null;
					}else if(root.firstChild == null && root.sibling != null){ // The current node does not have a firstChild but has a sibling
						prev.sibling = root.sibling;
						root.sibling = null;
						
					}else { // The current node does not have a firstChild and does not have a sibling
						prev.sibling = null;
					}
					
				}
				
				
			}
		
			
			pRemove(tag, tempChild, root);
			pRemove(tag, tempSibling, root);
		}
	}
	
	/**
	 * Adds a tag around all occurrences of a word in the DOM tree.
	 * 
	 * @param word Word around which tag is to be added
	 * @param tag Tag to be added
	 */
	public void addTag(String word, String tag) {
		/** COMPLETE THIS METHOD **/
		if(!(tag.equals("b") || tag.equals("em"))) {
			return;
		}
		
		
		addTagToNode(root, null, word, tag);
		
	}
	
	private void addTagToNode(TagNode mainT, TagNode prev, String word, String tag) {
		
		if(mainT == null) {
			return;
		}
		
		if(!(tag.equalsIgnoreCase("em") || tag.equalsIgnoreCase("b"))){
			return;
		}else {
			
		
			
			if(mainT.tag.equalsIgnoreCase(word)) { // word alone
				if(prev.firstChild == mainT) { // The word is the firstchild of the previous node
					if(mainT.sibling == null) { // The current node does not have a sibling
						prev.firstChild = new TagNode(tag, mainT, null);
						
					}else { // The current node does have a sibling
						
						prev.firstChild = new TagNode(tag, mainT, mainT.sibling);
						mainT.sibling = null;
					}
				}else { // The word is the sibling of the previous node
					
					//***********************
					
					if(mainT.sibling == null) { // The current node does not have a sibling
						prev.sibling = new TagNode(tag, mainT, null);
						
					}else { // The current node does have a sibling
						
						prev.sibling = new TagNode(tag, mainT, mainT.sibling);
						mainT.sibling = null;
					}
			
				}
				
			}else { // anything goes except the word is not alone => mainT.tag.length() > word
				if(mainT.tag.toUpperCase().indexOf(word.toUpperCase()) != -1) { // word is in current tag
					
					if(prev.firstChild == mainT) { // The word is the firstchild of the previous node
						addTagToCurrent1(prev, mainT, word, tag);
					}else { // The word is the sibling of the previous node
						addTagToCurrent2(prev, mainT, word, tag);
					}
				}
			}
			
			
			
			addTagToNode(mainT.firstChild, mainT, word, tag);
			addTagToNode(mainT.sibling, mainT, word, tag);
		}
	}
	
	/*
	 *  Adds the tag to all of the words in the current node
	 */
	private void addTagToCurrent1(TagNode prev, TagNode curr, String word, String tag) {
		TagNode result = null; // need to add to the END of result!!!!!! ******
		String finalWord = curr.tag;
		
		String current = curr.tag.toUpperCase(); // **
		word = word.toUpperCase(); // **
		
		while(current.indexOf(word) != -1) { // runs until the word is not in the current node
			
			
			if(result == null) { // Gives result a node to add siblings to 
				if(current.indexOf(word) == 0 && isAlone(finalWord, word)) { // The word is the first word in current and is Alone
					if(hasSyntax(current.charAt(word.length()))) { // Has syntax after the word
						result = new TagNode(tag, new TagNode(finalWord.substring(0, word.length()+1), null, null), null);	
						current = current.substring(word.length()+1);
						finalWord = finalWord.substring(word.length()+1);
					}else { // Does not have syntax after the word
						result = new TagNode(tag, new TagNode(finalWord.substring(0, word.length()), null, null), null);
						current = current.substring(word.length());
						finalWord = finalWord.substring(word.length());
						
					}
					
				}else { // The word is either not the first word in current or (but may or may not be alone)
					for(int ptr = current.indexOf(word); ptr != -1; ptr = current.indexOf(word, ptr+1)) {
						try {
							if(isAlone(finalWord.substring(ptr-1), word)) { // checks char before word if there is any words that are alone
								String previous = finalWord.substring(0, ptr);
								if((ptr + word.length()) == current.length()) { // The word is last and is Alone
									result = new TagNode(previous, null, new TagNode(tag, new TagNode(finalWord.substring(ptr), null, null), null));
									current = "";
									finalWord = "";
									break;
								}else {
									if(hasSyntax(current.charAt(ptr + word.length()))){ // Has syntax after word
										result = new TagNode(previous, null, new TagNode(tag,new TagNode(finalWord.substring(ptr, ptr+word.length()+1), null, null), null));
										try { 
											current = current.substring(ptr+word.length()+1);	
											finalWord = finalWord.substring(ptr+word.length()+1);
										}catch(Exception e) {
											current = "";
											finalWord = "";
										}
										
										break;
									}else { // Does not have have syntax after word
										result = new TagNode(previous, null, new TagNode(tag,new TagNode(finalWord.substring(ptr, ptr+word.length()), null, null), null));
										try {
											current = current.substring(ptr+word.length());	
											finalWord = finalWord.substring(ptr+word.length());	
										}catch(Exception e) {
											current = "";
											finalWord = "";
										}
										
										break;
									}
								}
								
							}
						}catch(Exception e) {
							continue;
						}
					}
					
				}
				
				if(result == null) {
					result = curr;
					current = "";
					finalWord = "";
					break;
				}
				
			}else { // Now to add to the result ************************************************************************************************
				if(current.indexOf(word) == 0 && isAlone(finalWord, word)) { // The word is the first word in current and is Alone
					if(hasSyntax(current.charAt(word.length()))) { // Has syntax after the word
						getLastSibling(result).sibling = new TagNode(tag, new TagNode(finalWord.substring(0, word.length()+1), null, null), null);	
						current = current.substring(word.length()+1);
						finalWord = finalWord.substring(word.length()+1);
					}else { // Does not have syntax after the word
						getLastSibling(result).sibling = new TagNode(tag, new TagNode(finalWord.substring(0, word.length()), null, null), null);
						current = current.substring(word.length());
						finalWord = finalWord.substring(word.length());
						
					}
					
				}else { // The word is either may or may not be the first word in current or (but may or may not be alone)
					for(int ptr = current.indexOf(word); ptr != -1; ptr = current.indexOf(word, ptr+1)) {
						try {
							if(isAlone(finalWord.substring(ptr-1), word)) { // checks char before word if there is any words that are alone
								String previous = finalWord.substring(0, ptr);
								
								if((ptr + word.length()) == current.length()) { // The word is last and is Alone
									getLastSibling(result).sibling = new TagNode(previous, null, new TagNode(tag, new TagNode(finalWord.substring(ptr), null, null), null));
									current = "";
									finalWord = "";
									break;
								}else {
									if(hasSyntax(current.charAt(ptr + word.length()))){ // Has syntax after word
										getLastSibling(result).sibling = new TagNode(previous, null, new TagNode(tag,new TagNode(finalWord.substring(ptr, ptr+word.length()+1), null, null), null));
										try { 
											current = current.substring(ptr+word.length()+1);	
											finalWord = finalWord.substring(ptr+word.length()+1);
										}catch(Exception e) {
											current = "";
											finalWord = "";
										}
										
										break;
									}else { // Does not have have syntax after word
										getLastSibling(result).sibling  = new TagNode(previous, null, new TagNode(tag,new TagNode(finalWord.substring(ptr, ptr+word.length()), null, null), null));
										try {
											current = current.substring(ptr+word.length());	
											finalWord = finalWord.substring(ptr+word.length());	
										}catch(Exception e) {
											current = "";
											finalWord = "";
										}
										
										break;
									}
								}
								
							}
						}catch(Exception e) {
							
							continue;
						}
					}
					
				}
				
				
			} // *******************************************************
			
			if(current.indexOf(word) != -1) {
				try {
					if(!isAlone(curr.tag.substring(current.indexOf(word)-1),word)) {
						break;
					}
				}catch(Exception e) {
					if(!isAlone(curr.tag.substring(current.indexOf(word)),word)) {
						break;
					}
				}
			}
		}
		
		if(finalWord != "") {
			getLastSibling(result).sibling = new TagNode(finalWord, null, null);
		}
		
		if(result == null) { // Means that there are no 'words's that are alone to add tags to
			prev.firstChild = curr;
		}
		
		
		if(curr.sibling == null) { // The current node does not have a sibling
			prev.firstChild = result;
			
		}else { // The current node does have a sibling
			TagNode temp = getLastSibling(result);
			getLastSibling(result).sibling = curr.sibling;
			prev.firstChild = result;
			curr.sibling = null;
			addTagToNode(temp, beforeWord(root,null,temp), word, tag);
		}
		
	}
	
	private void addTagToCurrent2(TagNode prev, TagNode curr, String word, String tag) {
		TagNode result = null; // need to add to the END of result!!!!!! ******
		String finalWord = curr.tag;
		
		String current = curr.tag.toUpperCase(); // **
		word = word.toUpperCase(); // **
		
		while(current.indexOf(word) != -1) { // runs until the word is not in the current node
			if(result == null) { // Gives result a node to add siblings to 
				if(current.indexOf(word) == 0 && isAlone(finalWord, word)) { // The word is the first word in current and is Alone
					if(hasSyntax(current.charAt(word.length()))) { // Has syntax after the word
						result = new TagNode(tag, new TagNode(finalWord.substring(0, word.length()+1), null, null), null);	
						current = current.substring(word.length()+1);
						finalWord = finalWord.substring(word.length()+1);
					}else { // Does not have syntax after the word
						result = new TagNode(tag, new TagNode(finalWord.substring(0, word.length()), null, null), null);
						current = current.substring(word.length());
						finalWord = finalWord.substring(word.length());
						
					}
					
				}else { // The word is either not the first word in current or (but may or may not be alone)
					for(int ptr = current.indexOf(word); ptr != -1; ptr = current.indexOf(word, ptr+1)) {
						try {
							if(isAlone(finalWord.substring(ptr-1), word)) { // checks char before word if there is any words that are alone
								String previous = finalWord.substring(0, ptr);
								if((ptr + word.length()) == current.length()) { // The word is last and is Alone
									result = new TagNode(previous, null, new TagNode(tag, new TagNode(finalWord.substring(ptr), null, null), null));
									current = "";
									finalWord = "";
									break;
								}else {
									if(hasSyntax(current.charAt(ptr + word.length()))){ // Has syntax after word
										result = new TagNode(previous, null, new TagNode(tag,new TagNode(finalWord.substring(ptr, ptr+word.length()+1), null, null), null));
										try { 
											current = current.substring(ptr+word.length()+1);	
											finalWord = finalWord.substring(ptr+word.length()+1);
										}catch(Exception e) {
											current = "";
											finalWord = "";
										}
										
										break;
									}else { // Does not have have syntax after word
										result = new TagNode(previous, null, new TagNode(tag,new TagNode(finalWord.substring(ptr, ptr+word.length()), null, null), null));
										try {
											current = current.substring(ptr+word.length());	
											finalWord = finalWord.substring(ptr+word.length());	
										}catch(Exception e) {
											current = "";
											finalWord = "";
										}
										
										break;
									}
								}
								
							}
						}catch(Exception e) {
							continue;
						}
					}
					
				}
				
				if(result == null) {
					result = curr;
					current = "";
					finalWord = "";
					break;
				}
				
			}else { // Now to add to the result ************************************************************************************************
				if(current.indexOf(word) == 0 && isAlone(finalWord, word)) { // The word is the first word in current and is Alone
					if(hasSyntax(current.charAt(word.length()))) { // Has syntax after the word
						getLastSibling(result).sibling = new TagNode(tag, new TagNode(finalWord.substring(0, word.length()+1), null, null), null);	
						current = current.substring(word.length()+1);
						finalWord = finalWord.substring(word.length()+1);
					}else { // Does not have syntax after the word
						getLastSibling(result).sibling = new TagNode(tag, new TagNode(finalWord.substring(0, word.length()), null, null), null);
						current = current.substring(word.length());
						finalWord = finalWord.substring(word.length());
						
					}
					
				}else { // The word is either may or may not be the first word in current or (but may or may not be alone)
					for(int ptr = current.indexOf(word); ptr != -1; ptr = current.indexOf(word, ptr+1)) {
						try {
							if(isAlone(finalWord.substring(ptr-1), word)) { // checks char before word if there is any words that are alone
								String previous = finalWord.substring(0, ptr);
								if((ptr + word.length()) == current.length()) { // The word is last and is Alone
									getLastSibling(result).sibling = new TagNode(previous, null, new TagNode(tag, new TagNode(finalWord.substring(ptr), null, null), null));
									current = "";
									finalWord = "";
									break;
								}else {
									if(hasSyntax(current.charAt(ptr + word.length()))){ // Has syntax after word
										getLastSibling(result).sibling = new TagNode(previous, null, new TagNode(tag,new TagNode(finalWord.substring(ptr, ptr+word.length()+1), null, null), null));
										try { 
											current = current.substring(ptr+word.length()+1);	
											finalWord = finalWord.substring(ptr+word.length()+1);
										}catch(Exception e) {
											current = "";
											finalWord = "";
										}
										
										break;
									}else { // Does not have have syntax after word
										getLastSibling(result).sibling  = new TagNode(previous, null, new TagNode(tag,new TagNode(finalWord.substring(ptr, ptr+word.length()), null, null), null));
										try {
											current = current.substring(ptr+word.length());	
											finalWord = finalWord.substring(ptr+word.length());	
										}catch(Exception e) {
											current = "";
											finalWord = "";
										}
										
										break;
									}
								}
								
							}
						}catch(Exception e) {
							
							continue;
						}
					}
					
				}
				
				
			} // *******************************************************
			if(current.indexOf(word) != -1) {
				try {
					if(!isAlone(curr.tag.substring(current.indexOf(word)-1),word)) {
						break;
					}
				}catch(Exception e) {
					if(!isAlone(curr.tag.substring(current.indexOf(word)),word)) {
						break;
					}
				}
			}
		}
		
		if(finalWord != "") {
			getLastSibling(result).sibling = new TagNode(finalWord, null, null);
		}
		
		if(result == null) { // Means that there are no 'words's that are alone to add tags to
			prev.firstChild = curr;
		}
		prev.sibling = result;
		
		if(curr.sibling == null) { // The current node does not have a sibling
			prev.sibling = result;
			
		}else { // The current node does have a sibling
			TagNode temp = getLastSibling(result);
			getLastSibling(result).sibling = curr.sibling;
			prev.sibling = result;
			curr.sibling = null;
			addTagToNode(temp, beforeWord(root,null,temp), word, tag);
		}
		
	}
	
	private static TagNode getLastSibling(TagNode n) {
		TagNode result = null;
		for(TagNode ptr = n; ptr != null; ptr = ptr.sibling) {
			result = ptr;
		}
		return result;
	}
	
	
	private static boolean hasSyntax(char c) {
		return c == '.' || c == ',' || c == '?' || c == '!' || c == ';' || c == ':';
	}
	
	/*
	 * returns true if the first instance of word is alone
	 */
	private static boolean isAlone(String tag, String word) {
		tag = tag.toUpperCase();
		word = word.toUpperCase();
		
		if(tag.equals(word)) {
			return true;
		}else if(tag.indexOf(word) == 0) {
			try {
				return checkFrontTwo(tag.substring(tag.indexOf(word) + word.length(), tag.indexOf(word) + word.length()+2));
			}catch(Exception e) {
				return checkFrontOne(tag.charAt(tag.indexOf(word)+word.length()));
			}
		}else {
			
			if((tag.indexOf(word)+word.length()) == tag.length()) {
				return checkBack(tag.charAt(tag.indexOf(word)-1));
				
			}
			
			try {
				boolean result = checkFrontTwo(tag.substring(tag.indexOf(word) + word.length(), tag.indexOf(word) + word.length()+2));
				
				try {
					result = checkBack(tag.charAt(tag.indexOf(word)-1));
					return result;
				}catch(Exception e) {
					return result;
				}
			}catch(Exception e) {
				boolean result = checkFrontOne(tag.charAt(tag.indexOf(word)+word.length()));
				
				try {
					result = checkBack(tag.charAt(tag.indexOf(word)-1));
					return result;
				}catch(Exception c) {
					return result;
				}
			}
		
		}
		
	}
	private static boolean checkFrontOne(char c) {
		return c == ' ' || c == '.' || c == ',' || c == '?' || c == '!' || c == ';' || c == ':';
	}
	private static boolean checkFrontTwo(String s) {
		if(checkFrontOne(s.charAt(0))) {
			char first = s.charAt(0);
			char second = s.charAt(1);
			if(second == ' ') { // " " + ' ' 
				return true;
			}
			if(first == ' ') { // ' ' + "doesn't matter"
				return true;
			}else {
				if(second == ' ') { // " "(not ' ') + ' ' 
					return true;
				}else if(second == '.' || second == ',' || second == '?' || second == '!' || second == ';' || second == ':') { // " "(not ' ') + ' '(all syntax)
					return false;
				}else { // " "(not ' ') + 'letters'
					return true;
				}
			}
		}
		return false;
		
	}
	private static boolean checkBack(char c) {
		return c == ' ';
	}	
	/*
	 * Finds the count'th occurrence of the word
	 * If the count'th occurrence of the word does not exist return null (does not work)
	 */
	/*
	private TagNode findWord(TagNode mainT, String word, int count) {
		
		
		TagNode result = null;
		if(mainT.tag.toUpperCase().indexOf(word.toUpperCase()) != -1 && count == 1) {
			return mainT;
		}
		
		if(mainT.tag.toUpperCase().indexOf(word.toUpperCase()) != -1 && count != 1) {
			count--;
		}
		
		if(result == null) {
			result = findWord(mainT.firstChild, word, count);
		}
		if(result == null) {
			result = findWord(mainT.sibling, word, count);
		}
		return result;
		
	}
	*/
	
	/*
	 * Returns the node before the given node
	 */
	private TagNode beforeWord(TagNode mainT, TagNode prev, TagNode wordT) {
		
		if(mainT == null) {
			return null;
		}
		
		TagNode result = null;
		
		if(mainT == wordT) {
			return prev;
		}
		
		prev = mainT;
		
		if(result == null) {
			result = beforeWord(mainT.firstChild, prev, wordT);
		}
		if(result == null) {
			result = beforeWord(mainT.sibling, prev, wordT);
		}
		
		return result;
	}
	
	/*
	 * Checks if the word is alone
	 */
	/*
	private int wordAlone(String wholeWord, String word) {
		wholeWord = wholeWord.toUpperCase();
		word = word.toUpperCase();
		if(wholeWord.length() == word.length() && wholeWord.equals(word)) {
			return 0;
		}
		
		if(wholeWord.indexOf(word) == 0) {
			try {
				
			}catch(Exception e){
				
			}
		}else {
			
		}
		
		
		if(wholeWord.indexOf(word) == 0) {
			if(wholeWord.charAt(wholeWord.indexOf(word)) == ' ') {
				return true;
			}
				
			if(wholeWord.charAt(wholeWord.indexOf(word)) == ',' || wholeWord.charAt(wholeWord.indexOf(word)) == '.' 
					|| wholeWord.charAt(wholeWord.indexOf(word)) == '!' || wholeWord.charAt(wholeWord.indexOf(word)) == '?' 
					|| wholeWord.charAt(wholeWord.indexOf(word)) == ' ') {
				if(wholeWord.indexOf(word)+1 > wholeWord.length()-1) {
					return true;
				}else {
					if(wholeWord.charAt(wholeWord.indexOf(word)+1) == ',' || wholeWord.charAt(wholeWord.indexOf(word)+1) == '.' 
							|| wholeWord.charAt(wholeWord.indexOf(word)+1) == '!' || wholeWord.charAt(wholeWord.indexOf(word)+1) == '?') {
						return false;
					}else {
						return true;
					}
					
				}
			}else {
				return false;
			}
		}else {
			if(wholeWord.charAt(wholeWord.indexOf(word)-1) == ' ' && wholeWord.charAt(wholeWord.indexOf(word)) == ' ') {
				return true;
			}
			
			
			
			
		}
		
		
		return -1;
	}
*/
	
	
	
	/**
	 * Gets the HTML represented by this DOM tree. The returned string includes
	 * new lines, so that when it is printed, it will be identical to the
	 * input file from which the DOM tree was built.
	 * 
	 * @return HTML string, including new lines. 
	 */
	public String getHTML() {
		StringBuilder sb = new StringBuilder();
		getHTML(root, sb);
		return sb.toString();
	}
	
	private void getHTML(TagNode root, StringBuilder sb) {
		for (TagNode ptr=root; ptr != null;ptr=ptr.sibling) {
			if (ptr.firstChild == null) {
				sb.append(ptr.tag);
				sb.append("\n");
			} else {
				sb.append("<");
				sb.append(ptr.tag);
				sb.append(">\n");
				getHTML(ptr.firstChild, sb);
				sb.append("</");
				sb.append(ptr.tag);
				sb.append(">\n");	
			}
		}
	}
	
	/**
	 * Prints the DOM tree. 
	 *
	 */
	public void print() {
		print(root, 1);
	}
	
	private void print(TagNode root, int level) {
		for (TagNode ptr=root; ptr != null;ptr=ptr.sibling) {
			for (int i=0; i < level-1; i++) {
				System.out.print("      ");
			};
			if (root != this.root) {
				System.out.print("|----");
			} else {
				System.out.print("     ");
			}
			System.out.println(ptr.tag);
			if (ptr.firstChild != null) {
				print(ptr.firstChild, level+1);
			}
		}
	}
}