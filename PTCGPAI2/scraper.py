# -*- coding: utf-8 -*-  # Add this line at the very top of your script

import requests
from bs4 import BeautifulSoup
import re
import csv

# Control variable to turn skipping messages on/off
verbose = False  # Set to False to suppress "skipping" messages

# Energy type mapping (key = class suffix, value = abbreviation)
energy_map = {
    "grass": "G",
    "fire": "F",
    "water": "W",
    "lightning": "L",
    "psychic": "P",
    "fighting": "I",
    "darkness": "D",
    "metal": "M",
    "colorless": "X"
}

type_map = {
    "Grass": "G",
    "Fire": "F",
    "Water": "W",
    "Lightning": "L",
    "Psychic": "P",
    "Fighting": "I",
    "Darkness": "D",
    "Metal": "M",
    "Colorless": "N",
    "Dragon": "R",
    }

# Function to fetch card URLs from the sitemap
def fetch_card_urls():
    sitemap_url = "https://www.pokemon-zone.com/card-sitemap.xml"
    response = requests.get(sitemap_url)

    # If the request was successful (status code 200), continue
    if response.status_code == 200:
        # Parse the XML content of the sitemap
        soup = BeautifulSoup(response.text, "xml")
        # Find all <loc> tags, which contain the URLs
        loc_tags = soup.find_all("loc")

        # Extract and return the URLs from the <loc> tags
        card_urls = [loc_tag.text.strip() for loc_tag in loc_tags]
        return card_urls
    else:
        print("Failed to retrieve the sitemap.")
        return []

# Function to scrape individual card data (name, for now)
def scrape_card_data(card_url):
    # Send a GET request to the card URL
    response = requests.get(card_url)

    # If the request was successful (status code 200), continue scraping
    if response.status_code == 200:
        # Parse the page content using BeautifulSoup
        soup = BeautifulSoup(response.text, "html.parser")

        # Check if the card is a Pokémon card by examining the header info
        header_info = soup.select_one("body > div.global-container > main > div > div > div.card-detail > div.card-detail__main > div.card-detail__content > div.heading-container.d-flex.justify-content-between.card-detail__header > div.flex-1 > div.fw-bold")

        if header_info:
                # Get the header text
                header_text = header_info.text.strip()

                # Use regex to check for any variation of "Pokémon"
                if re.search(r"Pok.*mon", header_text, re.IGNORECASE):

                    rarity_elements = soup.select("body > div.global-container > main > div > div > div.card-detail > div.card-detail__main > div.card-detail__content > div.heading-container.d-flex.justify-content-between.card-detail__header > div.flex-1 > div.d-flex.gap-2.align-items-center > span > span.rarity-icon__icon.rarity-icon__icon--diamond")
                
                    # If there's exactly 1 rarity element, proceed
                    if len(rarity_elements) == 1:

                        # Scrape card name based on the provided CSS selector
                        card_name_tag = soup.select_one("body > div.global-container > main > div > div > div.card-detail > div.card-detail__main > div.card-detail__content > div.heading-container.d-flex.justify-content-between.card-detail__header > div.flex-1 > div.d-flex.gap-2.align-items-center > h1")
            
                        # Check if the card name was found, and extract text
                        if card_name_tag:
                            card_name = re.sub(r'[^a-zA-Z]', '', card_name_tag.text.strip())
                        else:
                            card_name = "Card name not found"

                        # Scrape stage info from the same header (after "Pokémon")
                        stage_info = header_info.text.split('|')  # Split text at '|' to isolate the stage information
                        if len(stage_info) > 1:
                            stage_str = stage_info[1].strip()
    
                            # Map the stage string to an integer value
                            if stage_str.lower() == "basic":
                                stage = 0
                            elif stage_str.lower() == "stage 1":
                                stage = 1
                            elif stage_str.lower() == "stage 2":
                                stage = 2
                            else:
                                stage = -1  # For unknown stages or invalid data
                        else:
                            stage = -1  # In case no stage info is found
            
                        # Only proceed if the card is a "basic" card
                        if 0 == stage:

                            # Extract HP
                            hp_element = soup.select_one("div.fw-bold > span.fs-1.lh-1")
                            hp = hp_element.text.strip() if hp_element else "Unknown"

                            # Extract Pokémon Type
                            type_element = soup.select_one("div.d-flex.align-items-center.gap-1 > div > span")
                            if type_element:
                                type_class = type_element.get("class", [])
                                pokemon_type = "Unknown"
                                for cls in type_class:
                                    if "energy-icon--type-" in cls:
                                        type_name = cls.replace("energy-icon--type-", "")
                                        pokemon_type = type_map.get(type_name.capitalize(), "Unknown")
                                        break
                            else:
                                pokemon_type = "Unknown"

                            # Scrape attacks (can be 1 or 2 attacks)
                            attack_containers = soup.select("div.attack-summary-row__primary")  # Extract only "primary" rows

                            attacks = []
                            for attack in attack_containers:
                                # Extract attack name
                                attack_name_tag = attack.select_one("div.attack-summary-row__name")
                                attack_name = attack_name_tag.text.strip() if attack_name_tag else "Unknown Attack"

                                # Extract attack damage
                                attack_damage_tag = attack.select_one("div.attack-summary-row__damage")

                                if attack_damage_tag:
                                    attack_damage_raw = attack_damage_tag.text.strip()
    
                                    # Use regex to extract only the numeric part
                                    attack_damage_match = re.search(r"\d+", attack_damage_raw)
    
                                    if attack_damage_match:
                                        attack_damage = int(attack_damage_match.group())  # Convert to integer
                                    else:
                                        attack_damage = 0  # Default to 0 if no number is found
                                else:
                                    attack_damage = 0  # Default if no tag is found

                                # Extract energy cost from class attributes
                                energy_cost_tags = attack.select("div.attack-summary-row__costs span")
                                energy_cost = []
                                for tag in energy_cost_tags:
                                    class_attr = tag.get("class", [])  # Get the list of classes
                                    for class_name in class_attr:
                                        if "energy-icon--type-" in class_name:
                                            energy_type = class_name.replace("energy-icon--type-", "")
                                            mapped_energy = energy_map.get(energy_type, "?")  # Use mapping, fallback to '?'
                                            energy_cost.append(mapped_energy)

                                # Append attack data
                                attacks.append({
                                    "name": attack_name,
                                    "damage": attack_damage if attack_damage else 0,
                                    "energy_cost": energy_cost if energy_cost else ["No cost"]
                                })
                            # Extract Weakness
                            weakness_tag = soup.select_one("body > div.global-container > main > div > div > div.card-detail > div.card-detail__main > div.card-detail__content > div.card-detail__content-body > div.d-flex.gap-3.text-center.fw-bold > div:nth-child(1) > div.d-inline-flex.gap-1.fw-bold.align-items-center > div.w-24px > span")

                            if weakness_tag:
                                class_attr = weakness_tag.get("class", [])
                                weakness = "?"
                                for class_name in class_attr:
                                    if "energy-icon--type-" in class_name:
                                        energy_type = class_name.replace("energy-icon--type-", "")
                                        weakness = energy_map.get(energy_type, "?")
                            else:
                                weakness = "None"

                            retreat_cost_tags = soup.select("body > div.global-container > main > div > div > div.card-detail > div.card-detail__main > div.card-detail__content > div.card-detail__content-body > div.d-flex.gap-3.text-center.fw-bold > div:nth-child(2) > div.d-inline-flex.gap-2.align-items-center > div > span")
                            retreat_cost = len(retreat_cost_tags)  # Count number of energy icons

                            # Print the results
                            print(f"{card_name}")
                            print(f"HP: {hp} | Type: {pokemon_type} | Stage: {stage}")
                            print(f"Weakness: {weakness} | Retreat Cost: {retreat_cost}")
                            for attack in attacks:
                                energy_cost_str = "".join(attack["energy_cost"]).ljust(6)  # Left-aligned, 6 chars
                                attack_name_str = attack["name"][:20].ljust(20)  # Left-aligned, max 20 chars
                                damage_str = str(attack["damage"]).rjust(4)  # Right-aligned, 4 chars
    
                                print(f"{energy_cost_str}{attack_name_str}{damage_str}")
                            print()
                            # Return the card details
                            return {
                                "name": card_name,
                                "hp": hp,
                                "type": pokemon_type,
                                "stage": stage,
                                "weakness": weakness, 
                                "retreat_cost": retreat_cost,
                                "attacks": attacks
                            }
                        else:
                            if verbose:
                                print(f"Skipping non-basic card at {card_url}")
                    else:
                        if verbose:
                            print(f"Skipping card (incorrect rarity) at {card_url}")
                else:
                    if verbose:
                        print(f"Skipping non-Pokemon card at {card_url}")
        else:
            if verbose:
                print(f"Header info not found for {card_url}")

    else:
        if verbose:
            print(f"Failed to retrieve data for {card_url}")

# Function to write card data to a CSV file
def write_card_data_to_csv(cards, filename="pokemon_cards.csv"):
    with open(filename, mode="w", newline="", encoding="utf-8") as file:
        writer = csv.writer(file)
        
        # Write header, accounting for up to two attacks per card
        writer.writerow([
            "Card Name", "HP", "Type", "Stage", "Weakness", "Retreat Cost", 
            "Attack Name 1", "Attack Damage 1", "Energy Costs 1", 
            "Attack Name 2", "Attack Damage 2", "Energy Costs 2"
        ])

        # Write card data
        for card in cards:
            # For each attack, write the relevant details
            attack1 = card["attacks"][0] if len(card["attacks"]) > 0 else {}
            attack2 = card["attacks"][1] if len(card["attacks"]) > 1 else {}

            # Write the data for each card
            writer.writerow([
                card["name"], card["hp"], card["type"], card["stage"],
                card["weakness"], card["retreat_cost"], 
                attack1.get("name", ""), attack1.get("damage", ""), "".join(attack1.get("energy_cost", [])),
                attack2.get("name", ""), attack2.get("damage", ""), "".join(attack2.get("energy_cost", []))
            ])
        print(f"Data written to {filename}")

# Main function to scrape data for all cards and write to CSV
def main():
    card_urls = fetch_card_urls()
    cards = []

    for url in card_urls:  # For testing, limit to first 5 cards
        card_data = scrape_card_data(url)
        if card_data:
            cards.append(card_data)

    write_card_data_to_csv(cards)

if __name__ == "__main__":
    main()