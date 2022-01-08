package lse;

import java.io.*;
import java.util.*;

/**
 * This class builds an index of keywords. Each keyword maps to a set of pages in
 * which it occurs, with frequency of occurrence in each page.
 *
 */
public class LittleSearchEngine {
	
	/**
	 * This is a hash table of all keywords. The key is the actual keyword, and the associated value is
	 * an array list of all occurrences of the keyword in documents. The array list is maintained in 
	 * DESCENDING order of frequencies.
	 */
	HashMap<String,ArrayList<Occurrence>> keywordsIndex;
	
	/**
	 * The hash set of all noise words.
	 */
	HashSet<String> noiseWords;
	
	/**
	 * Creates the keyWordsIndex and noiseWords hash tables.
	 */
	public LittleSearchEngine() {
		keywordsIndex = new HashMap<String,ArrayList<Occurrence>>(1000,2.0f);
		noiseWords = new HashSet<String>(100,2.0f);
	}
	
	/**
	 * Scans a document, and loads all keywords found into a hash table of keyword occurrences
	 * in the document. Uses the getKeyWord method to separate keywords from other words.
	 * 
	 * @param docFile Name of the document file to be scanned and loaded
	 * @return Hash table of keywords in the given document, each associated with an Occurrence object
	 * @throws FileNotFoundException If the document file is not found on disk
	 */
	public HashMap<String,Occurrence> loadKeywordsFromDocument(String docFile) 
	throws FileNotFoundException {
		/** COMPLETE THIS METHOD **/
		Scanner sc = new Scanner(new File(docFile));
		HashMap<String, Occurrence> result = new HashMap<>();

		while(sc.hasNext()) {
			
			String curr = sc.next();
				
				if(!(getKeyword(curr) == "" || getKeyword(curr) == " ")) {
				
			
					if(getKeyword(curr) != null) {
						String tempS = getKeyword(curr);
						boolean isInHash = false;
						for(String s : result.keySet()) {
							if(s.equals(tempS)) {
								result.get(s).frequency++;
								isInHash = true;
							}
						}
						if(!isInHash) {
							Occurrence tempO = new Occurrence(docFile, 1);
							result.put(tempS, tempO);
						}
					}
				}
			
		}
		
		
		// following line is a placeholder to make the program compile
		// you should modify it as needed when you write your code
		return result;
	}
	
	/**
	 * Merges the keywords for a single document into the master keywordsIndex
	 * hash table. For each keyword, its Occurrence in the current document
	 * must be inserted in the correct place (according to descending order of
	 * frequency) in the same keyword's Occurrence list in the master hash table. 
	 * This is done by calling the insertLastOccurrence method.
	 * 
	 * @param kws Keywords hash table for a document
	 */
	public void mergeKeywords(HashMap<String,Occurrence> kws) { // Assuming all docs inputted are different??
		/** COMPLETE THIS METHOD **/
		
		for(String Hsub: kws.keySet()) {
			
			boolean isInHash = false;
			for(String Hmain: keywordsIndex.keySet()) {
				if(Hsub.equals(Hmain)) {
					keywordsIndex.get(Hmain).add(kws.get(Hsub));
					//if(Hsub.equals("simply")) {
						//System.out.println("********");
						//System.out.println(Hsub);
						//System.out.println(kws.get(Hsub).document);
						//System.out.println("********");
					//}
					insertLastOccurrence(keywordsIndex.get(Hmain));
					
					isInHash = true;
				}
			}
			if(!isInHash) {
				ArrayList<Occurrence> temp = new ArrayList<>();
				temp.add(kws.get(Hsub));
				keywordsIndex.put(Hsub, temp);
			}
			
		}
		
	}
	
	/**
	 * Given a word, returns it as a keyword if it passes the keyword test,
	 * otherwise returns null. A keyword is any word that, after being stripped of any
	 * trailing punctuation(s), consists only of alphabetic letters, and is not
	 * a noise word. All words are treated in a case-INsensitive manner.
	 * 
	 * Punctuation characters are the following: '.', ',', '?', ':', ';' and '!'
	 * NO OTHER CHARACTER SHOULD COUNT AS PUNCTUATION
	 * 
	 * If a word has multiple trailing punctuation characters, they must all be stripped
	 * So "word!!" will become "word", and "word?!?!" will also become "word"
	 * 
	 * See assignment description for examples
	 * 
	 * @param word Candidate word
	 * @return Keyword (word without trailing punctuation, LOWER CASE)
	 */
	public String getKeyword(String word) { // is the word rabbit' and rabbit" consider not all alphabetic characters??? How to deal with  true.)
		/** COMPLETE THIS METHOD **/
		String result = removeEnd(word).toLowerCase();
		
		if(isNoise(result)) {
			return null;
		}
		
		if(result.length() == 0) {
			return result;
		}
		
		
		for(int i = 0; i < result.length(); i++) {
			if(!isAlph(result.charAt(i))) {
				return null;
			}
		}
		
		return result;
	}
	
	/*
	 * @return true if the word is a noise word
	 */
	private boolean isNoise(String word) {
		boolean result = false;
		for(String s : noiseWords) {
			if(word.equalsIgnoreCase(s)) {
				return true;
			}
		}
		
		return result;
	}
	
	/*
	 * @return true if the char is an alphabetic letter
	 */
	private boolean isAlph(char c) {
		c = Character.toUpperCase(c);
		String alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		boolean result = false;
		
		for(int i = 0; i < 26; i++) {
			if(c == alphabet.charAt(i)) {
				result = true;
			}
		}
		
		return result;
	}
	
	/*
	 * @return true if the char is a listed punctuation
	 */
	private boolean isPunc(char c) {
		c = Character.toUpperCase(c);
		String punctuation = ".,:;?!";
		boolean result = false;
		
		for(int i = 0; i< 6; i++) {
			if(c == punctuation.charAt(i)) {
				return true;
			}
		}
		
		return result;
	}
	
	/*
	 * @return String with no punctuation at the end
	 */
	private String removeEnd(String s) {
		//System.out.println(s);
		if(!isPunc(s.charAt(s.length()-1))) {
			return s;
		}else {
			while(isPunc(s.charAt(s.length()-1))) {
				if(s.length() == 1) {
					return "";
				}
				s = s.substring(0, s.length()-1);
			}
		}
		
		return s;
	}
	/**
	 * Inserts the last occurrence in the parameter list in the correct position in the
	 * list, based on ordering occurrences on descending frequencies. The elements
	 * 0..n-2 in the list are already in the correct order. Insertion is done by
	 * first finding the correct spot using binary search, then inserting at that spot.
	 * 
	 * @param occs List of Occurrences
	 * @return Sequence of mid point indexes in the input list checked by the binary search process,
	 *         null if the size of the input list is 1. This returned array list is only used to test
	 *         your code - it is not used elsewhere in the program.
	 */
	public static ArrayList<Integer> insertLastOccurrence(ArrayList<Occurrence> occs) {
		/** COMPLETE THIS METHOD **/
		if(occs.size() == 1) {
			return null;
		}
		ArrayList<Integer> result = new ArrayList<>();
		int insert = occs.get(occs.size()-1).frequency;
		
		int l = 0, r = occs.size()-2;
		while(l <= r) {

			int m = l + (r-l) / 2;
			result.add(m);
			
			//System.out.println(l);
			//System.out.println(r);
			//System.out.println(m);
			
			if(occs.get(m).frequency == insert) {
				
				break;
			}
			
			if(occs.get(m).frequency > insert) {
				l = m+1;
			}else {
				r = m-1;
			}
			
		}
		
		int target = result.get(result.size()-1);
		Occurrence insertOcc = occs.get(occs.size()-1);
		occs.remove(occs.size()-1);
		
		//System.out.println(target);
		if(occs.get(target).frequency == insertOcc.frequency) {
			occs.add(target, insertOcc);
		}else if(occs.get(target).frequency < insertOcc.frequency) {
			occs.add(target, insertOcc);
		}else {
			occs.add(target+1, insertOcc);
		}
		
		/*
		ArrayList<Occurrence> tempO = keywordsIndex.get(Hmain);
		tempO.add(kws.get(Hsub));
		ArrayList<Integer> tempInt = insertLastOccurrence(tempO);
		int insertIndex = tempInt.get(tempInt.size()-1);
		
		if(tempO.get(insertIndex).frequency == kws.get(Hsub).frequency) {
			keywordsIndex.get(Hmain).add(insertIndex, kws.get(Hsub));
		}else if(tempO.get(insertIndex).frequency < kws.get(Hsub).frequency) {
			keywordsIndex.get(Hmain).add(insertIndex, kws.get(Hsub));
		}else {
			keywordsIndex.get(Hmain).add(insertIndex+1, kws.get(Hsub));
		}
		
		keywordsIndex.get(Hmain).remove(keywordsIndex.get(Hmain).size()-1);
		*/
		
		return result;
	}
	
	/**
	 * This method indexes all keywords found in all the input documents. When this
	 * method is done, the keywordsIndex hash table will be filled with all keywords,
	 * each of which is associated with an array list of Occurrence objects, arranged
	 * in decreasing frequencies of occurrence.
	 * 
	 * @param docsFile Name of file that has a list of all the document file names, one name per line
	 * @param noiseWordsFile Name of file that has a list of noise words, one noise word per line
	 * @throws FileNotFoundException If there is a problem locating any of the input files on disk
	 */
	public void makeIndex(String docsFile, String noiseWordsFile) 
	throws FileNotFoundException {
		// load noise words to hash table
		Scanner sc = new Scanner(new File(noiseWordsFile));
		while (sc.hasNext()) {
			String word = sc.next();
			noiseWords.add(word);
		}
		
		// index all keywords
		sc = new Scanner(new File(docsFile));
		while (sc.hasNext()) {
			String docFile = sc.next();
			HashMap<String,Occurrence> kws = loadKeywordsFromDocument(docFile);
			mergeKeywords(kws);
		}
		sc.close();
	}
	
	/*
	 * ******************************
	 * TEMP METHOD REMEMBER TO REMOVE
	 * ******************************
	 */
	public void fillNoise(String noiseWordsFile) throws FileNotFoundException {
		Scanner sc = new Scanner(new File(noiseWordsFile));
		while (sc.hasNext()) {
			String word = sc.next();
			noiseWords.add(word);
		}
		
	}
	
	/**
	 * Search result for "kw1 or kw2". A document is in the result set if kw1 or kw2 occurs in that
	 * document. Result set is arranged in descending order of document frequencies. 
	 * 
	 * Note that a matching document will only appear once in the result. 
	 * 
	 * Ties in frequency values are broken in favor of the first keyword. 
	 * That is, if kw1 is in doc1 with frequency f1, and kw2 is in doc2 also with the same 
	 * frequency f1, then doc1 will take precedence over doc2 in the result. 
	 * 
	 * The result set is limited to 5 entries. If there are no matches at all, result is null.
	 * 
	 * See assignment description for examples
	 * 
	 * @param kw1 First keyword
	 * @param kw1 Second keyword
	 * @return List of documents in which either kw1 or kw2 occurs, arranged in descending order of
	 *         frequencies. The result size is limited to 5 documents. If there are no matches, 
	 *         returns null or empty array list.
	 */
	public ArrayList<String> top5search(String kw1, String kw2) { // Assume makeIndex is called?
		/** COMPLETE THIS METHOD **/
		
		boolean k1 = false;
		boolean k2 = false;
		
		if(keywordsIndex.get(kw2) == null && keywordsIndex.get(kw1) != null) {
			k1 = true;
			k2 = false;
		}else if(keywordsIndex.get(kw2) != null && keywordsIndex.get(kw1) == null) {
			k1 = false;
			k2 = true;
		}else if(keywordsIndex.get(kw2) != null && keywordsIndex.get(kw1) != null) {
			k1 = true;
			k2 = true;
		}
			
			
		
		if(k1 == false && k2 == false) {
			return null;
		}
		
		ArrayList<String> result = new ArrayList<>();
		
		
		if((k1 != false && k2 != false) && (kw1.equals(kw2))) {
			ArrayList<Occurrence> temp = keywordsIndex.get(kw1);
			for(int i = 0; i < temp.size(); i++) {
				if(result.size() <= 5) {
					result.add(temp.get(i).document);
				}else {
					return result;
				}
			}
		}else {
		
			if(k1 == false && k2 != false) {
				ArrayList<Occurrence> temp = keywordsIndex.get(kw2);
				for(int i = 0; i < temp.size(); i++) {
					if(result.size() <= 5) {
						result.add(temp.get(i).document);
					}else {
						return result;
					}
				}
			}else if(k1 != false && k2 == false) {
				ArrayList<Occurrence> temp = keywordsIndex.get(kw1);
				for(int i = 0; i< temp.size(); i++) {
					if(result.size() <= 5) {
						result.add(temp.get(i).document);
					}else {
						return result;
					}
				}
			}else {
				ArrayList<Occurrence> temp1 = new ArrayList<>();
				ArrayList<Occurrence> temp2 = new ArrayList<>();
				
				for(int i = 0; i < keywordsIndex.get(kw1).size(); i++) {
					temp1.add(keywordsIndex.get(kw1).get(i));
				}
				for(int i = 0; i < keywordsIndex.get(kw2).size(); i++) {
					temp2.add(keywordsIndex.get(kw2).get(i));
				}
				
				
				ArrayList<Occurrence> resultO = isOrder(temp1, temp2);
				
				for(int i = 0; i < resultO.size(); i++) {
					if(result.size() <= 5) {
						boolean isDoc = false;
						
						for(int j = 0; j < result.size(); j++) {
							if(result.get(j).equals(resultO.get(i).document)) {
								isDoc = true;
							}
						}
						
						if(!isDoc) {
							result.add(resultO.get(i).document);
						}
					}else {
						return result;
					}
				}
				
			}
		
		}
		return result;
	
	}
	
	private ArrayList<Occurrence> isOrder(ArrayList<Occurrence> temp1, ArrayList<Occurrence> temp2){
		ArrayList<Occurrence> result = new ArrayList<>();
		
		boolean end = true;
		while(end) {

			if(temp1.size() == 0 && temp2.size() == 0) {
				break;
			}
			if(temp1.size() == 0 && temp2.size() != 0) {
				for(int i = 0; i < temp2.size(); i++) {
					result.add(temp2.get(i));
				}
				break;
			}
			if(temp1.size() != 0 && temp2.size() == 0) {
				for(int i = 0; i < temp1.size(); i++) {
					result.add(temp1.get(i));
				}
				break;
			}
			
			Occurrence first = temp1.get(temp1.size()-1);
			Occurrence second = temp2.get(temp2.size()-1);
			
			
			if(first.frequency == second.frequency) {
				result.add(first);
				temp1.remove(temp1.size()-1);
			}else if(first.frequency > second.frequency) {
				result.add(first);
				temp1.remove(temp1.size()-1);
			}else {
				result.add(second);
				temp2.remove(temp2.size()-1);
			}
		
		}
		return result;
		
	}
	
}